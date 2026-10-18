#include "ui/pickercontroller.h"

#include <gio/gio.h>

#include <array>
#include <optional>
#include <memory>
#include <utility>

namespace colourpicker {

namespace {

constexpr const char *portalBusName = "org.freedesktop.portal.Desktop";
constexpr const char *portalObjectPath = "/org/freedesktop/portal/desktop";
constexpr const char *screenshotInterface = "org.freedesktop.portal.Screenshot";
constexpr const char *requestInterface = "org.freedesktop.portal.Request";

std::shared_ptr<PickerController::State> stateFrom(void *data)
{
    return *static_cast<std::shared_ptr<PickerController::State> *>(data);
}

void releaseState(void *data)
{
    delete static_cast<std::shared_ptr<PickerController::State> *>(data);
}

void complete(const std::shared_ptr<PickerController::State> &state,
              const std::optional<Colour> &colour, const std::string &message);

} // namespace

struct PickerController::State final {
    explicit State(Completion callback)
        : completion(std::move(callback))
    {
    }

    Completion completion;
    GCancellable *cancellable{g_cancellable_new()};
    GDBusConnection *connection{nullptr};
    guint responseSubscription{0};
    std::string requestPath;
    bool isActive{false};

    ~State()
    {
        if (connection && responseSubscription != 0)
            g_dbus_connection_signal_unsubscribe(connection, responseSubscription);
        if (connection)
            g_object_unref(connection);
        g_object_unref(cancellable);
    }
};

namespace {

void complete(const std::shared_ptr<PickerController::State> &state,
              const std::optional<Colour> &colour, const std::string &message)
{
    if (!state->isActive)
        return;
    state->isActive = false;
    if (state->connection && state->responseSubscription != 0) {
        g_dbus_connection_signal_unsubscribe(state->connection, state->responseSubscription);
        state->responseSubscription = 0;
    }
    state->requestPath.clear();
    if (state->completion)
        state->completion(colour, message);
}

std::string senderPathPart(const char *uniqueName)
{
    std::string sender = uniqueName ? uniqueName : "";
    if (!sender.empty() && sender.front() == ':')
        sender.erase(0, 1);
    for (char &character : sender) {
        if (character == '.')
            character = '_';
    }
    return sender;
}

void responseReceived(GDBusConnection *, const gchar *, const gchar *, const gchar *, const gchar *,
                      GVariant *parameters, gpointer data)
{
    const auto state = stateFrom(data);
    guint response = 2;
    GVariant *results = nullptr;
    g_variant_get(parameters, "(u@a{sv})", &response, &results);
    if (response != 0) {
        g_variant_unref(results);
        complete(state, std::nullopt, "Colour picking was cancelled.");
        return;
    }

    gdouble red = 0.0;
    gdouble green = 0.0;
    gdouble blue = 0.0;
    const gboolean found = g_variant_lookup(results, "color", "(ddd)", &red, &green, &blue);
    g_variant_unref(results);
    if (!found) {
        complete(state, std::nullopt, "The desktop portal returned an invalid colour.");
        return;
    }
    const auto colour = colourFromPortal({red, green, blue});
    if (!colour) {
        complete(state, std::nullopt, "The desktop portal returned an invalid colour.");
        return;
    }
    complete(state, colour, "Colour picked.");
}

void pickColorFinished(GObject *source, GAsyncResult *result, gpointer data)
{
    const std::unique_ptr<std::shared_ptr<PickerController::State>> holder(
        static_cast<std::shared_ptr<PickerController::State> *>(data));
    const auto state = *holder;
    GError *error = nullptr;
    GVariant *reply = g_dbus_connection_call_finish(G_DBUS_CONNECTION(source), result, &error);
    if (!reply) {
        const std::string message = error ? error->message : "Could not start the desktop colour picker.";
        if (error)
            g_error_free(error);
        complete(state, std::nullopt, message);
        return;
    }

    const gchar *returnedPath = nullptr;
    g_variant_get(reply, "(&o)", &returnedPath);
    const std::string actualPath = returnedPath ? returnedPath : "";
    g_variant_unref(reply);
    if (actualPath.empty()) {
        complete(state, std::nullopt, "The desktop portal did not create a colour-picker request.");
        return;
    }
    if (actualPath != state->requestPath) {
        if (state->responseSubscription != 0)
            g_dbus_connection_signal_unsubscribe(state->connection, state->responseSubscription);
        state->requestPath = actualPath;
        state->responseSubscription = g_dbus_connection_signal_subscribe(
            state->connection, portalBusName, requestInterface, "Response", state->requestPath.c_str(), nullptr,
            G_DBUS_SIGNAL_FLAGS_NONE, responseReceived,
            new std::shared_ptr<PickerController::State>(state), releaseState);
    }
}

void busReady(GObject *, GAsyncResult *result, gpointer data)
{
    const std::unique_ptr<std::shared_ptr<PickerController::State>> holder(
        static_cast<std::shared_ptr<PickerController::State> *>(data));
    const auto state = *holder;
    GError *error = nullptr;
    state->connection = g_bus_get_finish(result, &error);
    if (!state->connection) {
        const std::string message = error ? error->message : "Could not connect to the desktop portal.";
        if (error)
            g_error_free(error);
        complete(state, std::nullopt, message);
        return;
    }

    const std::string token = "lgl_colour_picker_" + std::to_string(g_random_int());
    state->requestPath = "/org/freedesktop/portal/desktop/request/"
                         + senderPathPart(g_dbus_connection_get_unique_name(state->connection)) + "/" + token;
    state->responseSubscription = g_dbus_connection_signal_subscribe(
        state->connection, portalBusName, requestInterface, "Response", state->requestPath.c_str(), nullptr,
        G_DBUS_SIGNAL_FLAGS_NONE, responseReceived,
        new std::shared_ptr<PickerController::State>(state), releaseState);

    GVariantBuilder options;
    g_variant_builder_init(&options, G_VARIANT_TYPE_VARDICT);
    g_variant_builder_add(&options, "{sv}", "handle_token", g_variant_new_string(token.c_str()));
    g_dbus_connection_call(state->connection, portalBusName, portalObjectPath, screenshotInterface, "PickColor",
                           g_variant_new("(s@a{sv})", "", g_variant_builder_end(&options)), G_VARIANT_TYPE("(o)"),
                           G_DBUS_CALL_FLAGS_NONE, -1, state->cancellable, pickColorFinished,
                           new std::shared_ptr<PickerController::State>(state));
}

} // namespace

PickerController::PickerController(Completion completion)
    : m_state(std::make_shared<State>(std::move(completion)))
{
}

PickerController::~PickerController()
{
    m_state->completion = {};
    if (m_state->isActive && m_state->connection && !m_state->requestPath.empty()) {
        g_dbus_connection_call(m_state->connection, portalBusName, m_state->requestPath.c_str(), requestInterface,
                               "Close", nullptr, nullptr, G_DBUS_CALL_FLAGS_NONE, -1, nullptr, nullptr, nullptr);
    }
    g_cancellable_cancel(m_state->cancellable);
}

void PickerController::start()
{
    if (m_state->isActive)
        return;
    m_state->isActive = true;
    g_bus_get(G_BUS_TYPE_SESSION, m_state->cancellable, busReady,
              new std::shared_ptr<State>(m_state));
}

bool PickerController::active() const
{
    return m_state->isActive;
}

} // namespace colourpicker
