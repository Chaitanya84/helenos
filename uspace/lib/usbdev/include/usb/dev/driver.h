/*
 * Copyright (c) 2011 Vojtech Horky
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - The name of the author may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/** @addtogroup libusbdev
 * @{
 */
/** @file
 * USB device driver framework.
 */

#ifndef LIBUSBDEV_DRIVER_H_
#define LIBUSBDEV_DRIVER_H_

#include <usb/hc.h>
#include <usb/dev/alternate_ifaces.h>
#include <usb/dev/usb_device_connection.h>
#include <usb/dev/pipes.h>
#include <usb_iface.h>

/** USB device structure. */
typedef struct {
	/** Connection to USB hc, used by wire and arbitrary requests. */
	usb_hc_connection_t hc_conn;
	/** Connection backing the pipes.
	 * Typically, you will not need to use this attribute at all.
	 */
	usb_device_connection_t wire;
	/** The default control pipe. */
	usb_pipe_t ctrl_pipe;

	/** Other endpoint pipes.
	 * This is an array of other endpoint pipes in the same order as
	 * in usb_driver_t.
	 */
	usb_endpoint_mapping_t *pipes;
	/** Number of other endpoint pipes. */
	size_t pipes_count;
	/** Current interface.
	 * Usually, drivers operate on single interface only.
	 * This item contains the value of the interface or -1 for any.
	 */
	int interface_no;
	/** Alternative interfaces. */
	usb_alternate_interfaces_t alternate_interfaces;

	/** Some useful descriptors for USB device. */
	struct {
		/** Standard device descriptor. */
		usb_standard_device_descriptor_t device;
		/** Full configuration descriptor of current configuration. */
		const uint8_t *configuration;
		size_t configuration_size;
	} descriptors;

	/** Generic DDF device backing this one. DO NOT TOUCH! */
	ddf_dev_t *ddf_dev;
	/** Custom driver data.
	 * Do not use the entry in generic device, that is already used
	 * by the framework.
	 */
	void *driver_data;

	usb_dev_session_t *bus_session;
} usb_device_t;

/** USB driver ops. */
typedef struct {
	/** Callback when a new device was added to the system. */
	int (*device_add)(usb_device_t *);
	/** Callback when a device is about to be removed from the system. */
	int (*device_rem)(usb_device_t *);
	/** Callback when a device was removed from the system. */
	int (*device_gone)(usb_device_t *);
} usb_driver_ops_t;

/** USB driver structure. */
typedef struct {
	/** Driver name.
	 * This name is copied to the generic driver name and must be exactly
	 * the same as the directory name where the driver executable resides.
	 */
	const char *name;
	/** Expected endpoints description.
	 * This description shall exclude default control endpoint (pipe zero)
	 * and must be NULL terminated.
	 * When only control endpoint is expected, you may set NULL directly
	 * without creating one item array containing NULL.
	 *
	 * When the driver expect single interrupt in endpoint,
	 * the initialization may look like this:
\code
static usb_endpoint_description_t poll_endpoint_description = {
	.transfer_type = USB_TRANSFER_INTERRUPT,
	.direction = USB_DIRECTION_IN,
	.interface_class = USB_CLASS_HUB,
	.interface_subclass = 0,
	.interface_protocol = 0,
	.flags = 0
};

static usb_endpoint_description_t *hub_endpoints[] = {
	&poll_endpoint_description,
	NULL
};

static usb_driver_t hub_driver = {
	.endpoints = hub_endpoints,
	...
};
\endcode
	 */
	const usb_endpoint_description_t **endpoints;
	/** Driver ops. */
	const usb_driver_ops_t *ops;
} usb_driver_t;

int usb_driver_main(const usb_driver_t *);

int usb_device_init(usb_device_t *, ddf_dev_t *,
    const usb_endpoint_description_t **, const char **);
void usb_device_deinit(usb_device_t *);

const char* usb_device_get_name(usb_device_t *);
ddf_fun_t *usb_device_ddf_fun_create(usb_device_t *, fun_type_t, const char *);

async_exch_t * usb_device_bus_exchange_begin(usb_device_t *);
void usb_device_bus_exchange_end(async_exch_t *);

int usb_device_select_interface(usb_device_t *, uint8_t,
    const usb_endpoint_description_t **);

int usb_device_create_pipes(usb_device_t *usb_dev,
    const usb_endpoint_description_t **endpoints);
void usb_device_destroy_pipes(usb_device_t *);

usb_pipe_t *usb_device_get_default_pipe(usb_device_t *);
usb_endpoint_mapping_t * usb_device_get_mapped_ep_desc(usb_device_t *,
    const usb_endpoint_description_t *);
usb_endpoint_mapping_t * usb_device_get_mapped_ep(usb_device_t *,
    usb_endpoint_t);

int usb_device_get_iface_number(usb_device_t *);

const usb_standard_device_descriptor_t *
usb_device_get_device_descriptor(usb_device_t *);
const void * usb_device_get_configuration_descriptor(usb_device_t *, size_t *);
const usb_alternate_interfaces_t * usb_device_get_alternative_ifaces(
    usb_device_t *);

void * usb_device_data_alloc(usb_device_t *, size_t);
void * usb_device_data_get(usb_device_t *);

#endif
/**
 * @}
 */
