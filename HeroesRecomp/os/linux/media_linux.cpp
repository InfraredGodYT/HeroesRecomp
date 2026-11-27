#include <os/media.h>
#include <os/logger.h>
#include <user/config.h>
#include <dbus/dbus.h>

static constexpr auto POLL_INTERVAL = std::chrono::milliseconds(250);
static constexpr int CALL_TIMEOUT_MS = 500;

static std::atomic<bool> g_isPlaying;
static std::once_flag g_pollThreadOnce;

static bool IsPlayerPlaying(DBusConnection* connection, const char* name)
{
    const char* interfaceName = "org.mpris.MediaPlayer2.Player";
    const char* propertyName = "PlaybackStatus";

    DBusMessage* message = dbus_message_new_method_call(name, "/org/mpris/MediaPlayer2", DBUS_INTERFACE_PROPERTIES, "Get");
    dbus_message_append_args(message, DBUS_TYPE_STRING, &interfaceName, DBUS_TYPE_STRING, &propertyName, DBUS_TYPE_INVALID);

    DBusMessage* reply = dbus_connection_send_with_reply_and_block(connection, message, CALL_TIMEOUT_MS, nullptr);
    dbus_message_unref(message);

    if (!reply)
    {
        return false;
    }

    bool isPlaying = false;
    DBusMessageIter args;
    DBusMessageIter variant;

    if (dbus_message_iter_init(reply, &args) && dbus_message_iter_get_arg_type(&args) == DBUS_TYPE_VARIANT)
    {
        dbus_message_iter_recurse(&args, &variant);

        if (dbus_message_iter_get_arg_type(&variant) == DBUS_TYPE_STRING)
        {
            const char* status;
            dbus_message_iter_get_basic(&variant, &status);
            isPlaying = strcmp(status, "Playing") == 0;
        }
    }

    dbus_message_unref(reply);
    return isPlaying;
}

static bool QueryIsPlaying(DBusConnection* connection)
{
    DBusMessage* message = dbus_message_new_method_call(DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS, "ListNames");
    DBusMessage* reply = dbus_connection_send_with_reply_and_block(connection, message, CALL_TIMEOUT_MS, nullptr);
    dbus_message_unref(message);

    if (!reply)
    {
        return false;
    }

    bool isPlaying = false;
    char** names;
    int count;

    if (dbus_message_get_args(reply, nullptr, DBUS_TYPE_ARRAY, DBUS_TYPE_STRING, &names, &count, DBUS_TYPE_INVALID))
    {
        for (int i = 0; i < count && !isPlaying; i++)
        {
            if (std::string_view(names[i]).starts_with("org.mpris.MediaPlayer2."))
            {
                isPlaying = IsPlayerPlaying(connection, names[i]);
            }
        }

        dbus_free_string_array(names);
    }

    dbus_message_unref(reply);
    return isPlaying;
}

static void PollThread()
{
    DBusError error;
    dbus_error_init(&error);

    DBusConnection* connection = dbus_bus_get_private(DBUS_BUS_SESSION, &error);
    if (!connection)
    {
        LOGF_ERROR("Music Attenuation: Failed to connect to D-Bus session bus: {}", error.message);
        dbus_error_free(&error);
        return;
    }

    dbus_connection_set_exit_on_disconnect(connection, false);
    while (dbus_connection_get_is_connected(connection))
    {
        g_isPlaying = Config::MusicAttenuation && QueryIsPlaying(connection);
        while (DBusMessage* message = dbus_connection_pop_message(connection))
        {
            dbus_message_unref(message);
        }
        std::this_thread::sleep_for(POLL_INTERVAL);
    }
    LOG_ERROR("Music Attenuation: Lost connection to D-Bus session bus");
    g_isPlaying = false;
    dbus_connection_close(connection);
    dbus_connection_unref(connection);
}

bool os::media::IsSupported()
{
    // Most systems should support this, and we can't detect the flatpak permission so defaulting to enabled
    return true;
}

bool os::media::IsExternalMediaPlaying()
{
    std::call_once(g_pollThreadOnce, []
        {
            std::thread(PollThread).detach();
        });

    return g_isPlaying;
}
