/* Fit-to-width -- Fit zoom to the image width
 *
 * Copyright (C) 2009 The Free Software Foundation
 *
 * Author: Javier Sánchez  <jsanchez@deskblue.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gi18n-lib.h>
#include <xviewer/xviewer-scroll-view.h>
#include <xviewer/xviewer-image.h>
#include <xviewer/xviewer-window.h>
#include <xviewer/xviewer-window-activatable.h>

#include <libpeas/peas.h>

#include "xviewer-fit-to-width-plugin.h"


static void
xviewer_window_activatable_iface_init (XviewerWindowActivatableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (XviewerFitToWidthPlugin, xviewer_fit_to_width_plugin,
		PEAS_TYPE_EXTENSION_BASE, 0,
		G_IMPLEMENT_INTERFACE_DYNAMIC (XVIEWER_TYPE_WINDOW_ACTIVATABLE,
					xviewer_window_activatable_iface_init))

enum {
	PROP_0,
	PROP_WINDOW
};

static void
fit_to_width_cb (GtkAction *action, XviewerWindow *window)
{
	GtkWidget     *view;
	XviewerImage      *image;
	gint           image_width;
	gint           image_height;
	gint           view_width;
	double         zoom;
	GtkAllocation  allocation;


	g_return_if_fail (XVIEWER_IS_WINDOW (window));

	view = xviewer_window_get_view (window);
	image = xviewer_window_get_image (window);

	g_return_if_fail (XVIEWER_IS_SCROLL_VIEW (view));
	g_return_if_fail (XVIEWER_IS_IMAGE (image));

	xviewer_image_get_size (image, &image_width, &image_height);
	gtk_widget_get_allocation (view, &allocation);
	view_width = allocation.width;

	// HACK: It's necessary subtract the width size (15) of vscrollbar
	//       to scrollview for obtain the display area.
	zoom = (double) (view_width - 15) / image_width;

	xviewer_scroll_view_set_zoom (XVIEWER_SCROLL_VIEW (view), zoom);
}

static const gchar * const ui_definition =
	"<ui><menubar name=\"MainMenu\">"
	"<menu action=\"View\">"
	"<menuitem action=\"XviewerPluginFitToWidth\"/>"
	"</menu></menubar></ui>";

static const GtkActionEntry action_entries[] =
{
	{ "XviewerPluginFitToWidth",
	  "zoom-fit-best",
	  N_("Fit to width"),
	  "W",
	  N_("Fit the image to the window width"),
	  G_CALLBACK (fit_to_width_cb) }
};


static void
xviewer_fit_to_width_plugin_init (XviewerFitToWidthPlugin *plugin)
{
	plugin->ui_action_group = NULL;
	plugin->ui_menuitem_id = 0;
}

static void
impl_activate (XviewerWindowActivatable *activatable)
{
	XviewerFitToWidthPlugin *plugin = XVIEWER_FIT_TO_WIDTH_PLUGIN (activatable);
	GtkUIManager *manager;

	manager = xviewer_window_get_ui_manager (plugin->window);

	plugin->ui_action_group = gtk_action_group_new ("XviewerFitToWidthPluginActions");

	gtk_action_group_set_translation_domain (plugin->ui_action_group,
						 GETTEXT_PACKAGE);

	gtk_action_group_add_actions (plugin->ui_action_group,
				      action_entries,
				      G_N_ELEMENTS (action_entries),
				      plugin->window);

	gtk_ui_manager_insert_action_group (manager,
					    plugin->ui_action_group,
					    -1);

	plugin->ui_menuitem_id = gtk_ui_manager_add_ui_from_string (manager,
								  ui_definition,
								  -1, NULL);
}

static void
impl_deactivate	(XviewerWindowActivatable *activatable)
{
	XviewerFitToWidthPlugin *plugin = XVIEWER_FIT_TO_WIDTH_PLUGIN (activatable);
	GtkUIManager *manager;

	manager = xviewer_window_get_ui_manager (plugin->window);

	gtk_ui_manager_remove_ui (manager,
				  plugin->ui_menuitem_id);

	plugin->ui_menuitem_id = 0;

	gtk_ui_manager_remove_action_group (manager,
					    plugin->ui_action_group);
	g_object_unref (plugin->ui_action_group);
	plugin->ui_action_group = NULL;
}

static void
xviewer_fit_to_width_plugin_dispose (GObject *object)
{
	XviewerFitToWidthPlugin *plugin = XVIEWER_FIT_TO_WIDTH_PLUGIN (object);

	if (plugin->window != NULL) {
		g_object_unref (plugin->window);
		plugin->window = NULL;
	}

	G_OBJECT_CLASS (xviewer_fit_to_width_plugin_parent_class)->dispose (object);
}

static void
xviewer_fit_to_width_plugin_get_property (GObject    *object,
				      guint       prop_id,
				      GValue     *value,
				      GParamSpec *pspec)
{
	XviewerFitToWidthPlugin *plugin = XVIEWER_FIT_TO_WIDTH_PLUGIN (object);

	switch (prop_id)
	{
	case PROP_WINDOW:
		g_value_set_object (value, plugin->window);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
xviewer_fit_to_width_plugin_set_property (GObject      *object,
				      guint         prop_id,
				      const GValue *value,
				      GParamSpec   *pspec)
{
	XviewerFitToWidthPlugin *plugin = XVIEWER_FIT_TO_WIDTH_PLUGIN (object);

	switch (prop_id)
	{
	case PROP_WINDOW:
		plugin->window = XVIEWER_WINDOW (g_value_dup_object (value));
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
xviewer_fit_to_width_plugin_class_init (XviewerFitToWidthPluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = xviewer_fit_to_width_plugin_dispose;
	object_class->set_property = xviewer_fit_to_width_plugin_set_property;
	object_class->get_property = xviewer_fit_to_width_plugin_get_property;

	g_object_class_override_property (object_class, PROP_WINDOW, "window");
}

static void
xviewer_fit_to_width_plugin_class_finalize (XviewerFitToWidthPluginClass *klass)
{
	/* Dummy needed for G_DEFINE_DYNAMIC_TYPE_EXTENDED */
}

static void
xviewer_window_activatable_iface_init (XviewerWindowActivatableInterface *iface)
{
	iface->activate = impl_activate;
	iface->deactivate = impl_deactivate;
}

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
	xviewer_fit_to_width_plugin_register_type (G_TYPE_MODULE (module));
	peas_object_module_register_extension_type (module,
						    XVIEWER_TYPE_WINDOW_ACTIVATABLE,
						    XVIEWER_TYPE_FIT_TO_WIDTH_PLUGIN);
}
