#include <dbus/dbus.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

void log_event(const char* message) {
    time_t now;
    time(&now);
    FILE* log_file = fopen("event_monitor.log", "a");
    if (log_file) {
        fprintf(log_file, "[%s] %s\n", ctime(&now), message);
        fclose(log_file);
    }
}

int main() {
    DBusError error;
    DBusConnection* connection;
    
    dbus_error_init(&error);
    connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
    
    if (dbus_error_is_set(&error)) {
        log_event("DBus connection error");
        fprintf(stderr, "Connection Error: %s\n", error.message);
        dbus_error_free(&error);
        return 1;
    }
    
    // Мониторинг системных событий
    dbus_bus_add_match(connection,
        "type='signal',interface='org.freedesktop.DBus'",
        &error);
    
    while (1) {
        dbus_connection_read_write(connection, 0);
        DBusMessage* msg = dbus_connection_pop_message(connection);
        
        if (msg != NULL) {
            char* msg_info;
            dbus_message_get_args(msg, &error,
                DBUS_TYPE_STRING, &msg_info,
                DBUS_TYPE_INVALID);
            
            if (!dbus_error_is_set(&error)) {
                char log_msg[256];
                snprintf(log_msg, sizeof(log_msg),
                    "Event received: %s", msg_info);
                log_event(log_msg);
            }
            
            dbus_message_unref(msg);
        }
        
        sleep(1);
    }
    
    dbus_connection_unref(connection);
    return 0;
}
