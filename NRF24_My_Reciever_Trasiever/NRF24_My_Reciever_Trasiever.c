/*
 * NRF24_My_Reciever_Trasiever.c
 *
 * Created: 23.03.2016 10:12:26
 *  Author: AndreyMaznyak
 */ 


#include <avr/io.h>
#include "Radio.h"
#include "WriteProgramm.h"

#define NOT_CHECK 0
#define UPDATE_PROGRAM 1
#define PUT_PAGE 2
#define PAGE_NUMBER 3
#define GET_PAGE_DATA 4

uint8_t status = 0;
uint32_t page = 0;

// Вызывается, когда превышено число попыток отправки, а подтверждение так и не было получено.
void on_send_error_IRQ() {
	// TODO здесь можно описать обработчик неудачной отправки
	PORTD &= ~(1<<7);
	_delay_ms(300);
}

// Вызывается при получении нового пакета по каналу 1 от удалённой стороны.
// buf - буфер с данными, size - длина данных (от 1 до 32)
void on_packet_IRQ(uint8_t * buf, uint8_t size) {
	// TODO здесь нужно написать обработчик принятого пакета

	// Если предполагается немедленная отправка ответа, то необходимо обеспечить задержку ,
	// во время которой чип отправка подтверждение о приёме
	// чтобы с момента приёма пакета до перевода в режим PTX прошло:
	// 130мкс + ((длина_адреса + длина_CRC + длина_данных_подтверждения) * 8 + 17) / скорость_обмена
	// При типичных условиях и частоте МК 8 мГц достаточно дополнительной задержки 100мкс
	_delay_ms(200);
	if(status == GET_PAGE_DATA){
		//boot_program_page(page,buf);	
	}
	if(buf[0] == PAGE_NUMBER){
		page = (buf[4] << 24) | (buf[3] << 16) | (buf[2] << 8) | buf[1];
		status = GET_PAGE_DATA; 
	}
	//Запись страницы
	send_data(buf, size);
}


ISR(INT0_vect)
{
	cli();
	PORTD|= 1<<7;
	uint8_t status = radio_cmd(NOP);
	radio_writereg(STATUS, status); // Просто запишем регистр обратно, тем самым сбросив биты прерываний
	
	if (status & ((1 << TX_DS) | (1 << MAX_RT))) { // Завершена передача успехом, или нет,
		if (status & (1 << MAX_RT)) { // Если достигнуто максимальное число попыток
			radio_cmd(FLUSH_TX); // Удалим последний пакет из очереди
			on_send_error_IRQ(); // Вызовем обработчик
		}
		if (!(radio_readreg(FIFO_STATUS) & (1 << TX_EMPTY))) { // Если в очереди передатчика есть что передавать
			radio_assert_ce(); // Импульс на линии CE приведёт к началу передачи
			_delay_us(15); // Нужно минимум 10мкс, возьмём с запасом
			radio_deassert_ce();
			} else {
			uint8_t conf = radio_readreg(CONFIG);
			radio_writereg(CONFIG, conf | (1 << PRIM_RX)); // Устанавливаем бит PRIM_RX: приём
			radio_assert_ce(); // Высокий уровень на линии CE переводит радио-чип в режим приёма
		}
	}
	uint8_t protect = 4; // В очереди FIFO не должно быть более 3 пакетов. Если больше, значит что-то не так
	while (((status & (7 << RX_P_NO)) != (7 << RX_P_NO)) && protect--) { // Пока в очереди есть принятый пакет
		uint8_t l = radio_read_rx_payload_width(); // Узнаём длину пакета
		if (l > 32) { // Ошибка. Такой пакет нужно сбросить
			radio_cmd(FLUSH_RX);
			} else {
			uint8_t buf[32]; // буфер для принятого пакета
			radio_read_buf(R_RX_PAYLOAD, &buf[0], l); // начитывается пакет
			if ((status & (7 << RX_P_NO)) == (1 << RX_P_NO)) { // если datapipe 1
				on_packet_IRQ(&buf[0], l); // вызываем обработчик полученного пакета
			}
		}
		status = radio_cmd(NOP);
	}
	PORTD	&= ~(1<<7);
	sei();
}


int main(void)
{
    
    //Светодиоды
    //led started
    DDRD	|= 1<<6;
    DDRD	|= 1<<7;
    
    PORTD	|= 1<<6;
    _delay_ms(1000);
	//_delay_ms(10000);
    //PORTD	&= ~(1<<6);
    //_delay_ms(1000);
    //PORTD	|= 1<<6;
    //_delay_ms(1000);
    //PORTD	&= ~(1<<6);
    //_delay_ms(1000);
    
    radio_init();
    	
    while (!radio_start()) {
	    PORTD	&= ~(1<<6);
	    _delay_ms(1000);
    }
    PORTD	|= 1<<6;
    
	
	//настраиваем на срабатывание INT0 по переднему фронту
	MCUCR &= ~( (1<<ISC11)|(1<<ISC10)|(1<<ISC01)|(1<<ISC00) );
	//разрешаем внешнее прерывание INT0
	GICR |= (1<<INT0);
	
	sei();
    
	// Перед включением питания чипа и сигналом CE должно пройти время достаточное для начала работы осциллятора
    // Для типичных резонаторов с эквивалентной индуктивностью не более 30мГн достаточно 1.5 мс
    _delay_ms(2);
    
    radio_assert_ce();
    
    //uint8_t data = 0;//[2] = {0,1};//,3};//,2,3,4,5,6,7,8,9,10,11,12,13,14,15};//,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
	
    while(1) {
	    //data++;
	    //send_data(&data, sizeof(data));
	    //_delay_us(500);
	    //check_radio();
	   
		//_delay_ms(300);
	    // TODO здесь основной код программы
	    
	    /*if(data > 254){
		    data = 0;
			//data[1]++;
			
		    radio_init();
		    
		    while (!radio_start()) {
			    PORTD	&= ~(1<<6);
			    PORTD	&= ~(1<<7); 
				_delay_ms(1000);
		    }
		    PORTD	|= 1<<6;
		    
	    }*/
    }
}