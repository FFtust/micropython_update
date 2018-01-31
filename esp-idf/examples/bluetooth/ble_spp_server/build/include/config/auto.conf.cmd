deps_config := \
	/d/fftust/esp32-separate/esp-idf/components/app_trace/Kconfig \
	/d/fftust/esp32-separate/esp-idf/components/aws_iot/Kconfig \
	/d/fftust/esp32-separate/esp-idf/components/bt/Kconfig \
	/d/fftust/esp32-separate/esp-idf/components/esp32/Kconfig \
	/d/fftust/esp32-separate/esp-idf/components/ethernet/Kconfig \
	/d/fftust/esp32-separate/esp-idf/components/fatfs/Kconfig \
	/d/fftust/esp32-separate/esp-idf/components/freertos/Kconfig \
	/d/fftust/esp32-separate/esp-idf/components/heap/Kconfig \
	/d/fftust/esp32-separate/esp-idf/components/libsodium/Kconfig \
	/d/fftust/esp32-separate/esp-idf/components/log/Kconfig \
	/d/fftust/esp32-separate/esp-idf/components/lwip/Kconfig \
	/d/fftust/esp32-separate/esp-idf/components/mbedtls/Kconfig \
	/d/fftust/esp32-separate/esp-idf/components/openssl/Kconfig \
	/d/fftust/esp32-separate/esp-idf/components/pthread/Kconfig \
	/d/fftust/esp32-separate/esp-idf/components/spi_flash/Kconfig \
	/d/fftust/esp32-separate/esp-idf/components/spiffs/Kconfig \
	/d/fftust/esp32-separate/esp-idf/components/tcpip_adapter/Kconfig \
	/d/fftust/esp32-separate/esp-idf/components/wear_levelling/Kconfig \
	/d/fftust/esp32-separate/esp-idf/components/bootloader/Kconfig.projbuild \
	/d/fftust/esp32-separate/esp-idf/components/esptool_py/Kconfig.projbuild \
	/d/fftust/esp32-separate/esp-idf/components/partition_table/Kconfig.projbuild \
	/d/fftust/esp32-separate/esp-idf/Kconfig

include/config/auto.conf: \
	$(deps_config)


$(deps_config): ;
