PREFIX                  = /usr
INCLUDE_DIR             = ${PREFIX}/include/reactcpp
LIBRARY_DIR             = ${PREFIX}/lib

all:
		$(MAKE) -C src all

static:
		$(MAKE) -C src all

shared:
		$(MAKE) -C src shared

clean:
		$(MAKE) -C src clean

install:
		mkdir -p ${INCLUDE_DIR}/amqp
		mkdir -p ${LIBRARY_DIR}
		cp -f amqp.h ${INCLUDE_DIR}
		cp -f include/*.h ${INCLUDE_DIR}/amqp
		cp -f src/libreactcpp-amqp.so $(LIBRARY_DIR)
		cp -f src/libreactcpp-amqp.a $(LIBRARY_DIR)
