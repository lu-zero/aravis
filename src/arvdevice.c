/* Aravis - Digital camera library
 *
 * Copyright © 2009-2010 Emmanuel Pacaud
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Author: Emmanuel Pacaud <emmanuel@gnome.org>
 */

/** 
 * SECTION: arvdevice
 * @short_description: Abstract base class for device handling
 *
 * #ArvDevice is an abstract base class for the control of cameras. It provides
 * an easy access to the camera settings, and to its genicam interface for  more
 * advanced uses.
 */

#include <arvdevice.h>
#include <arvgc.h>
#include <arvgccommand.h>
#include <arvgcinteger.h>
#include <arvgcfloat.h>
#include <arvgcboolean.h>
#include <arvgcenumeration.h>
#include <arvgcstring.h>
#include <arvstream.h>

enum {
	ARV_DEVICE_SIGNAL_CONTROL_LOST,
	ARV_DEVICE_SIGNAL_LAST
} ArvDeviceSignals;

static guint arv_device_signals[ARV_DEVICE_SIGNAL_LAST] = {0};

static GObjectClass *parent_class = NULL;

/**
 * arv_device_create_stream:
 * @device: a #ArvDevice
 * @callback: (scope call): a frame processing callback
 * @user_data: (closure) user data for @callback
 * Return value: (transfer full): a new #ArvStream.
 *
 * Creates a new #ArvStream for video stream handling. See
 * @ArvStreamCallback for details regarding the callback function.
 */

ArvStream *
arv_device_create_stream (ArvDevice *device, ArvStreamCallback callback, void *user_data)
{
	g_return_val_if_fail (ARV_IS_DEVICE (device), NULL);

	return ARV_DEVICE_GET_CLASS (device)->create_stream (device, callback, user_data);
}

gboolean
arv_device_read_memory (ArvDevice *device, guint32 address, guint32 size, void *buffer)
{
	g_return_val_if_fail (ARV_IS_DEVICE (device), FALSE);
	g_return_val_if_fail (buffer != NULL, FALSE);
	g_return_val_if_fail (size > 0, FALSE);

	return ARV_DEVICE_GET_CLASS (device)->read_memory (device, address, size, buffer);
}

gboolean
arv_device_write_memory (ArvDevice *device, guint32 address, guint32 size, void *buffer)
{
	g_return_val_if_fail (ARV_IS_DEVICE (device), FALSE);
	g_return_val_if_fail (buffer != NULL, FALSE);
	g_return_val_if_fail (size > 0, FALSE);

	return ARV_DEVICE_GET_CLASS (device)->write_memory (device, address, size, buffer);
}

gboolean
arv_device_read_register (ArvDevice *device, guint32 address, guint32 *value)
{
	g_return_val_if_fail (ARV_IS_DEVICE (device), FALSE);
	g_return_val_if_fail (value != NULL, FALSE);

	return ARV_DEVICE_GET_CLASS (device)->read_register (device, address, value);
}

gboolean
arv_device_write_register (ArvDevice *device, guint32 address, guint32 value)
{
	g_return_val_if_fail (ARV_IS_DEVICE (device), FALSE);

	return ARV_DEVICE_GET_CLASS (device)->write_register (device, address, value);
}

/**
 * arv_device_get_genicam: 
 * @device: a device object 
 * Return value: (transfer none): the genicam interface.
 *
 * Retrieves the genicam interface of the given device.
 */

ArvGc *
arv_device_get_genicam (ArvDevice *device)
{
	g_return_val_if_fail (ARV_IS_DEVICE (device), NULL);

	return ARV_DEVICE_GET_CLASS (device)->get_genicam (device);
}

static const char *
_get_genicam_xml (ArvDevice *device, size_t *size)
{
	*size = 0;

	return NULL;
}

/**
 * arv_device_get_xml:
 * @device: a #ArvDevice
 * @size: placeholder for the returned data size (bytes)
 * Return value: a pointer to the Genicam XML data, owned by the device.
 *
 * Gets the Genicam XML data stored in the device memory.
 **/

const char *
arv_device_get_genicam_xml (ArvDevice *device, size_t *size)
{
	g_return_val_if_fail (ARV_IS_DEVICE (device), NULL);
	g_return_val_if_fail (size != NULL, NULL);

	return ARV_DEVICE_GET_CLASS (device)->get_genicam_xml (device, size);
}

void
arv_device_execute_command (ArvDevice *device, const char *feature)
{
	ArvGc *genicam;
	ArvGcNode *node;

	genicam = arv_device_get_genicam (device);
	g_return_if_fail (ARV_IS_GC (genicam));

	node = arv_gc_get_node (genicam, feature);
	if (ARV_IS_GC_COMMAND (node))
		arv_gc_command_execute (ARV_GC_COMMAND (node));
}

/**
 * arv_device_get_feature:
 * @device: a #ArvDevice
 * @feature: a feature name
 *
 * Return value: (transfer none): the genicam node corresponding to the feature name, NULL if not found.
 */

ArvGcNode *
arv_device_get_feature (ArvDevice *device, const char *feature)
{
	ArvGc *genicam;

	genicam = arv_device_get_genicam (device);
	g_return_val_if_fail (ARV_IS_GC (genicam), NULL);

	return arv_gc_get_node (genicam, feature);
}

void
arv_device_set_string_feature_value (ArvDevice *device, const char *feature, const char *value)
{
	ArvGcNode *node;

	node = arv_device_get_feature (device, feature);

	if (ARV_IS_GC_ENUMERATION (node))
		arv_gc_enumeration_set_string_value (ARV_GC_ENUMERATION (node), value);
	else if (ARV_IS_GC_STRING (node))
		arv_gc_string_set_value (ARV_GC_STRING (node), value);
}

const char *
arv_device_get_string_feature_value (ArvDevice *device, const char *feature)
{
	ArvGcNode *node;

	node = arv_device_get_feature (device, feature);

	if (ARV_IS_GC_ENUMERATION (node))
		return arv_gc_enumeration_get_string_value (ARV_GC_ENUMERATION (node));
	else if (ARV_IS_GC_STRING (node))
		return arv_gc_string_get_value (ARV_GC_STRING (node));

	return NULL;
}

void
arv_device_set_integer_feature_value (ArvDevice *device, const char *feature, gint64 value)
{
	ArvGcNode *node;

	node = arv_device_get_feature (device, feature);

	if (ARV_IS_GC_INTEGER (node))
		arv_gc_integer_set_value (ARV_GC_INTEGER (node), value);
	else if (ARV_IS_GC_ENUMERATION (node))
		arv_gc_enumeration_set_int_value (ARV_GC_ENUMERATION (node), value);
	else if (ARV_IS_GC_BOOLEAN (node))
		arv_gc_boolean_set_value (ARV_GC_BOOLEAN (node), value);
}

gint64
arv_device_get_integer_feature_value (ArvDevice *device, const char *feature)
{
	ArvGcNode *node;

	node = arv_device_get_feature (device, feature);

	if (ARV_IS_GC_INTEGER (node))
		return arv_gc_integer_get_value (ARV_GC_INTEGER (node));
	else if (ARV_IS_GC_ENUMERATION (node))
		return arv_gc_enumeration_get_int_value (ARV_GC_ENUMERATION (node));
	else if (ARV_IS_GC_BOOLEAN (node))
		return arv_gc_boolean_get_value (ARV_GC_BOOLEAN (node));

	return 0;
}

void
arv_device_get_integer_feature_bounds (ArvDevice *device, const char *feature, gint64 *min, gint64 *max)
{
	ArvGcNode *node;

	node = arv_device_get_feature (device, feature);

	if (ARV_IS_GC_INTEGER (node)) {
		if (min != NULL)
			*min = arv_gc_integer_get_min (ARV_GC_INTEGER (node));
		if (max != NULL)
			*max = arv_gc_integer_get_max (ARV_GC_INTEGER (node));
		return;
	}
}

void
arv_device_set_float_feature_value (ArvDevice *device, const char *feature, double value)
{
	ArvGcNode *node;

	node = arv_device_get_feature (device, feature);

	if (ARV_IS_GC_FLOAT (node))
		arv_gc_float_set_value (ARV_GC_FLOAT (node), value);
}

double
arv_device_get_float_feature_value (ArvDevice *device, const char *feature)
{
	ArvGcNode *node;

	node = arv_device_get_feature (device, feature);

	if (ARV_IS_GC_FLOAT (node))
		return arv_gc_float_get_value (ARV_GC_FLOAT (node));

	return 0.0;
}

void
arv_device_get_float_feature_bounds (ArvDevice *device, const char *feature, double *min, double *max)
{
	ArvGcNode *node;

	node = arv_device_get_feature (device, feature);

	if (ARV_IS_GC_FLOAT (node)) {
		if (min != NULL)
			*min = arv_gc_float_get_min (ARV_GC_FLOAT (node));
		if (max != NULL)
			*max = arv_gc_float_get_max (ARV_GC_FLOAT (node));
	}
}

void
arv_device_emit_control_lost_signal (ArvDevice *device)
{
	g_return_if_fail (ARV_IS_DEVICE (device));

	g_signal_emit (device, arv_device_signals[ARV_DEVICE_SIGNAL_CONTROL_LOST], 0);
}

static void
arv_device_init (ArvDevice *device)
{
}

static void
arv_device_finalize (GObject *object)
{
	parent_class->finalize (object);
}

static void
arv_device_class_init (ArvDeviceClass *device_class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (device_class);

	parent_class = g_type_class_peek_parent (device_class);

	object_class->finalize = arv_device_finalize;

	device_class->get_genicam_xml = _get_genicam_xml;

	/**
	 * ArvDevice::control-lost:
	 * @device:a #ArvDevice
	 *
	 * Signal that the control of the device is lost.
	 *
	 * This signal may be emited from a thread different than the main one,
	 * so please take care to shared data access from the callback.
	 */

	arv_device_signals[ARV_DEVICE_SIGNAL_CONTROL_LOST] =
		g_signal_new ("control-lost",
			      G_TYPE_FROM_CLASS (device_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (ArvDeviceClass, control_lost),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0, G_TYPE_NONE);
}

G_DEFINE_ABSTRACT_TYPE (ArvDevice, arv_device, G_TYPE_OBJECT)
