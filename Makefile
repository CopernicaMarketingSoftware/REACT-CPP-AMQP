PREFIX                  = /usr
INCLUDE_DIR             = ${PREFIX}/include/reactcpp

install:
		mkdir -p ${INCLUDE_DIR}/amqp
		cp -f amqp.h ${INCLUDE_DIR}
		cp -f include/*.h ${INCLUDE_DIR}/amqp
