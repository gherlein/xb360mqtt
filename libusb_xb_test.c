#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libusb-1.0/libusb.h>


void printdev(libusb_device *dev) {
struct libusb_device_descriptor desc;
struct libusb_config_descriptor *config;
struct libusb_interface_descriptor *interdesc;
struct libusb_endpoint_descriptor *epdesc;
struct libusb_interface *inter;
uint i,j,k;

        libusb_get_device_descriptor(dev, &desc);

        printf("Vendor ID: %x\n", desc.idVendor);
        printf("Product ID: %x\n", desc.idProduct);

        libusb_get_config_descriptor(dev, 0, &config);

        for(i=0; i<(int)config->bNumInterfaces; i++) {
                inter = &config->interface[i];
                printf("Number of alternate settings: %d\n", inter->num_altsetting);
                for(j=0; j<inter->num_altsetting; j++) {
                        interdesc = &inter->altsetting[j];
                        printf("Interface Number: %d\n", (int)interdesc->bInterfaceNumber);
                        printf("Number of endpoints: %d\n", (int)interdesc->bNumEndpoints);
                        for(k=0; k<(int)interdesc->bNumEndpoints; k++) {
                                epdesc = &interdesc->endpoint[k];
                                printf("Descriptor Type: %d\n", (int)epdesc->bDescriptorType);
                                printf("EP Address: %d\n", (int)epdesc->bEndpointAddress);
                        }
                }
        }
}


int main(int argc, char **argv) {
libusb_context *ctx = NULL;
uint r, cnt, x;
libusb_device **devs;
libusb_device_handle *dev_handle;
unsigned char data[512];
uint actual;

        printf("[Xbox Controller Test]: @_xpn_\n\n");

        memset(data, 0xFF, sizeof(data));

        r = libusb_init(&ctx);

        if (r < 0) {
                printf("[X] Error init libusb\n");
                return 1;
        }

        libusb_set_debug(ctx, 3);

        cnt = libusb_get_device_list(ctx, &devs);

        if (cnt < 0) {
                printf("[X] libusb_get_device_list failed\n");
                return 1;
        }

        for(x=0; x < cnt; x++) {
                printdev(devs[x]);
                printf("-----------------------\n");
        }


        // Grab a handle to our XBONE controller
//        dev_handle = libusb_open_device_with_vid_pid(ctx, 0x45e, 0x02d1);
        dev_handle = libusb_open_device_with_vid_pid(ctx, 0x45e, 0x028e);

        if (dev_handle == NULL) {
                printf("[X] Cannot open device, ensure XBOX controller is attached\n");
                return 1;
        }

        if (libusb_kernel_driver_active(dev_handle, 0) == 1) {
                printf("[i] Kernel has hold of this device, detaching kernel driver\n");
                libusb_detach_kernel_driver(dev_handle, 0);
        }

        libusb_claim_interface(dev_handle, 0);

        // Start our rumble test
        // Thanks to https://github.com/quantus/xbox-one-controller-protocol

        printf("[!] Sending Rumble Test 1.. LT\n");
        libusb_interrupt_transfer(dev_handle, 0x01, "\x09\x08\x00\x09\x00\x0f\x20\x04\x20\x20\xFF\x00", 12, &actual, 5000);

        sleep(3);

        printf("[!] Sending Rumble Test 1.. RT\n");
        libusb_interrupt_transfer(dev_handle, 0x01, "\x09\x08\x00\x09\x00\x0f\x04\x20\x20\x20\xFF\x00", 12, &actual, 5000);

        while(1) {
                libusb_interrupt_transfer(dev_handle, 0x81, data, sizeof(data), &actual, 5000);

//                printf("Received %d bytes\n", actual);
                for(x=0; x < actual; x++) {
                        printf("%02X ", data[x]);
                }
                printf("\n");
        }


        libusb_release_interface(dev_handle, 0);
}
