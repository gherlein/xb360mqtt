

mqtt:
	gcc -o m mqtt_example.c -lmosquitto

clean:
	rm -Rf *~ x



x:
	gcc -o x ./libusb_xb_test.c -lusb-1.0 -L/usr/local/lib

