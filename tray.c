#include <time.h>

#include <gtk/gtk.h>
#include <gio/gio.h>

#include "eyerest-dbus.h"

static GtkWidget* menu = NULL;
static GtkWidget* menu_item_state = NULL;
static GtkStatusIcon *tray_icon = NULL;
static OrgZlbruceEyerestBasic* eye_proxy = NULL;

static void on_delay (GtkStatusIcon* status_icon, gpointer user_data)
{
    guint delay_time = GPOINTER_TO_UINT(user_data);

    GError* error = NULL;

    if (!org_zlbruce_eyerest_basic_call_delay_sync (
                eye_proxy,
                delay_time,
                NULL,
                &error))
    {
        g_print ("call dbus methed delay failed: %s\n", error->message);
        return;
    }
}

static void on_pause (GtkStatusIcon* status_icon, gpointer user_data)
{
    GError* error = NULL;
    if (!org_zlbruce_eyerest_basic_call_pause_sync (
                eye_proxy,
                NULL,
                &error))
    {
        g_print ("call dbus methed pause failed: %s\n", error->message);
        return;
    }
}

static void on_unpause (GtkStatusIcon* status_icon, gpointer user_data)
{
    GError* error = NULL;
    if (!org_zlbruce_eyerest_basic_call_unpause_sync (
                eye_proxy,
                NULL,
                &error))
    {
        g_print ("call dbus methed unpause failed: %s\n", error->message);
        return;
    }
}

static void on_rest_now (GtkStatusIcon* status_icon, gpointer user_data)
{
    GError* error = NULL;
    if (!org_zlbruce_eyerest_basic_call_rest_now_sync (
                eye_proxy,
                NULL,
                &error))
    {
        g_print ("call dbus methed rest_now failed: %s\n", error->message);
        return;
    }
}

static void on_quit (GtkStatusIcon* status_icon, gpointer user_data)
{
    gtk_main_quit();
}

static GtkWidget* create_menu()
{
    GtkWidget* menu;

    menu = gtk_menu_new();

    menu_item_state = gtk_menu_item_new_with_label("State: ");
    gtk_widget_set_sensitive(menu_item_state, FALSE);
    GtkWidget* delay3 = gtk_menu_item_new_with_label("delay 3 min");
    GtkWidget* delay5 = gtk_menu_item_new_with_label("delay 5 min");
    GtkWidget* pause = gtk_menu_item_new_with_label("pause");
    GtkWidget* unpause = gtk_menu_item_new_with_label("continue");
    GtkWidget* rest_now = gtk_menu_item_new_with_label("rest now");
    GtkWidget* quit = gtk_menu_item_new_with_label("quit");
    GtkWidget* sep1 = gtk_separator_menu_item_new();
    GtkWidget* sep2 = gtk_separator_menu_item_new();
    GtkWidget* sep3 = gtk_separator_menu_item_new();

    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item_state);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), sep1);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), delay3);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), delay5);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), sep2);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), pause);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), unpause);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), rest_now);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), sep3);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), quit);

    g_signal_connect (G_OBJECT(delay3), "activate",
            G_CALLBACK(on_delay), GUINT_TO_POINTER(180));
    g_signal_connect (G_OBJECT(delay5), "activate",
            G_CALLBACK(on_delay), GUINT_TO_POINTER(300));
    g_signal_connect (G_OBJECT(pause), "activate",
            G_CALLBACK(on_pause), NULL);
    g_signal_connect (G_OBJECT(unpause), "activate",
            G_CALLBACK(on_unpause), NULL);
    g_signal_connect (G_OBJECT(rest_now), "activate",
            G_CALLBACK(on_rest_now), NULL);
    g_signal_connect (G_OBJECT(quit), "activate",
            G_CALLBACK(on_quit), NULL);

    gtk_widget_show_all(menu);

    return menu;
}


void tray_icon_on_click(GtkStatusIcon *status_icon, 
        gpointer user_data)
{
    gtk_menu_popup(GTK_MENU(menu), NULL, NULL, gtk_status_icon_position_menu, status_icon, 0, gtk_get_current_event_time());
}

void tray_icon_on_menu(GtkStatusIcon *status_icon, guint button, 
        guint activate_time, gpointer user_data)
{
    gtk_menu_popup(GTK_MENU(menu), NULL, NULL, gtk_status_icon_position_menu, status_icon, button, activate_time);
}

static GtkStatusIcon *create_tray_icon() 
{
    GtkStatusIcon *tray_icon;

    tray_icon = gtk_status_icon_new();
    g_signal_connect(G_OBJECT(tray_icon), "activate", 
            G_CALLBACK(tray_icon_on_click), NULL);
    g_signal_connect(G_OBJECT(tray_icon), "popup-menu",
            G_CALLBACK(tray_icon_on_menu), NULL);
    gtk_status_icon_set_from_icon_name(tray_icon, "stock_appointment-reminder");
    gtk_status_icon_set_title(tray_icon, "Eyerest");
    gtk_status_icon_set_visible(tray_icon, TRUE);

    return tray_icon;
}


void on_status (
        OrgZlbruceEyerestBasic *object,
        guint arg_time_remain,
        const gchar *arg_state)
{
    time_t time_remain = arg_time_remain;
    static gchar time_str[PATH_MAX];
    struct tm* tm = localtime(&time_remain);
    if(tm == NULL)
    {
        g_snprintf(time_str, sizeof(time_str), "%lu", (unsigned long)time);
    }
    else
    {
        strftime(time_str, sizeof(time_str), "%M:%S", tm);
    }

    //g_printf("time_str = %s, state = %s\n", time_str, arg_state);

    gtk_menu_item_set_label(GTK_MENU_ITEM(menu_item_state), time_str);
    gtk_status_icon_set_tooltip_text (tray_icon, time_str);
}

int main(int argc, char *argv[]) 
{
    gtk_init(&argc, &argv);

    GError* error = NULL;

    eye_proxy = org_zlbruce_eyerest_basic_proxy_new_for_bus_sync(
            G_BUS_TYPE_SESSION,
            G_DBUS_PROXY_FLAGS_NONE,
            "org.zlbruce.eyerest",
            "/",
            NULL,
            &error);
    if(eye_proxy == NULL)
    {
        g_print ("init dbus failed: %s\n", error->message);
        return -1;
    }

    g_signal_connect(G_OBJECT(eye_proxy), "status",
            G_CALLBACK(on_status), NULL);

    tray_icon = create_tray_icon();
    menu = create_menu();

    gtk_main();

    g_object_unref (eye_proxy);
    return 0;
}
