# test_CAN

Тестирование CAN-шины на stm32f4_discovery.
Установка фильтров, скорость 250кГц.   
STM32CubeMX 6.01, Keil 5.27              

Микроконтроллер принимает данные по USB и отправляет их по CAN шине. 
(в stm32f1xx USB и CAN не могут работать одновременно, в отличие от stm32f3xx) 

Для подключение к компьютеру использовал Robotel USB-CAN
Программа опроса находится на https://github.com/nopnop2002/Robotell-USB-CAN-Python  

![Подключение](https://github.com/fiskov/testProg/blob/master/test_bxCAN/test_bxCAN_filter.jpg)  

