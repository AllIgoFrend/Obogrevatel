#include "../main/termostat.h"
static uint8_t pwr_state = 0U;
static uint8_t invers = 1U;
static esp_err_t err = 0U;
static os_timer_t power_termsns;
static char bufer[9] ={0};
static uint16_t termostat = 0U;
static uint16_t kT = 0U;
static time_t strt_pls = 0U;
static time_t strt_mns = 0U;
static time_t strt_lohi = 0U;
static time_t strt_pwr = 0U;
static time_t heat_time = 0U;
static time_t current_time = 0U;
static os_timer_t timer_buzz;
static uint8_t fp = 3U;
static uint8_t state_mem = 89U;
static uint8_t k = 0U;
static int8_t c = 0U;
static uint8_t d = 0U;
static uint16_t t = 0U;
static uint8_t celevaya = 0U;
static uint8_t delta = 0U;
static int8_t t_on = 0U;
static int8_t t_off = 0U;
static os_timer_t timer_press;

typedef struct {
    u8_t tim_flg:1;
    u8_t rel_on:1;
    u8_t start_init_f:1;
    u8_t mess_on:1;
    u8_t forse_on:1;
    u8_t rezerv:3;
} flgs;
flgs flagi = {0};
static uint32_t keys = GPIO_NUM_MAX;
//------------------------------------------------------adc i on off power na termosens
static int8_t *celoe()
{
  c = temper_massiv[termostat]/10U;
	return &c;
}
static uint8_t *drobnoe()
{
  d = temper_massiv[termostat]%10U;
	return &d;
}
static uint16_t *kelvin(int8_t *temp)
{
  t = *temp + KELVIN_0;
	return &t;
}
void ICACHE_FLASH_ATTR izmer()
{
  err = gpio_set_level(MESSURE, (1U & GET_BIT(flagi.mess_on)));
    if(err != ESP_OK){
        printf("mesure power en err %s\n ", esp_err_to_name(err));
    }	
}
static void ICACHE_FLASH_ATTR adc_timer()
{
	err = adc_read(&termostat);
		if(err != ESP_OK){
			printf("ацп_err: %d\n",err);
		}else{
			termostat -= DEG_50;
			snprintf(bufer, sizeof (bufer), "%i.%d", *celoe(), *drobnoe());
			kT = *kelvin(celoe());
		}
	CLIRE_BIT(flagi.mess_on);
	izmer();
}
// гистерезис +30...+25 гистерезис 0/-2, +24...+20 гистерезис 0/-1, +20...+10 гист +1/-1, +10...+5 гист +1/0

static void hysteresis(bool on)
{
  if(on){
    if(celevaya > 24U){
      t_off = celevaya;
      t_on = celevaya - 2U;
    } else if(celevaya > 19U && celevaya < 25U){
      t_off  = celevaya;
      t_on = celevaya - 1U;
    } else if(celevaya > 9U && celevaya < 20U){
      t_off  = celevaya + 1U;
      t_on = celevaya - 1U;
    } else if(celevaya > 4U && celevaya < 10U){
      t_off  = celevaya + 1U;
      t_on = celevaya;
    }
  }else{
    t_off = t_on = celevaya;
  }
}

static uint8_t *delta_t()
{
  if(GET_BIT(flagi.rel_on)){
    if((kT != 0U) && (*kelvin(&t_off) > kT)){
      k = *kelvin(&t_off) - kT;
    } else {
      k = 0U;
    }
  } else {
    if((kT != 0U) && (*kelvin(&t_on) > kT)){
      k = *kelvin(&t_off) - kT;
    } else {
      k = 0U;
    }
  }
  hysteresis(true);
  return &k;
}
//------------------------------------------------------gpio

static void ICACHE_FLASH_ATTR timer_func_user()
{
  pwm_stop(0x00);
}

void ICACHE_FLASH_ATTR out_init()
{
    gpio_config_t pGPIOConfig = {
        .intr_type = GPIO_INTR_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = RELE_1_PIN | RELE_2_PIN | MESSURE_PIN
    };
	err = gpio_config(&pGPIOConfig);
    if(err != ESP_OK){
        printf("gpio konfig err %s\n ", esp_err_to_name(err));
    }
}

void ICACHE_FLASH_ATTR buzz_pwm_init()
{
	 /* Инициализировать ШИМ, период, рабочий цикл pwm_duty_init, 3 канала, канал ввода-вывода */
  uint32_t dutic = 150U;
	uint32_t io_info[] ={BUZZER};
	uint32_t pwm_duty_init[] = {dutic};
  uint32_t period = 450U;
 // pwm_init (1000, pwm_duty_init, 3, io_info); // цикл равен 1000us, что соответствует частоте 1 кГц, начальный рабочий цикл каждого канала равен 0
 
	 //pwm_init (450, &pwm_duty_init[0], 1, &io_info[0]); // Занять только 1 канал, установить IO 13
	err = pwm_init (period, &pwm_duty_init[0], 1U, &io_info[0]); // Занять только 1 канал, установить IO 13
    if(err != ESP_OK){
        printf("pwm init err %s\n ", esp_err_to_name(err));
    }
	 /* Сбрасываем рабочий цикл. */
	err = pwm_set_duty (0U, dutic); // 0 ~ 2 (три канала)
    if(err != ESP_OK){
        printf("pwm set duti %s\n ", esp_err_to_name(err));
    }
	err = pwm_set_period(period);  // set period
    if(err != ESP_OK){
        printf("pwm set period %s\n ", esp_err_to_name(err));
    }
  err = pwm_set_phase(0U,0);
    if(err != ESP_OK){
        printf("pwm faza %s\n ", esp_err_to_name(err));
    }	
	err = pwm_start();
    if(err != ESP_OK){
        printf("pwm start %s\n ", esp_err_to_name(err));
    }	
}
static void rele_inv(uint8_t *uslovie)
{
  if(*uslovie == 3U || *uslovie == 0U){
    if(invers == 0U || invers >= 2U){
      invers = 1U;
    }else{
      invers = 2U;
    }
  }
}
static void ICACHE_FLASH_ATTR debonce_f()
{
  if(!gpio_get_level(keys)){
    if(keys == POWER_SET_NUM){
      time(&strt_pwr);
    }
    if(keys == LO_HI_NUM){
      if(pwr_state < 3U){
        pwr_state += 1U;
      }else{
        pwr_state = 1U;
      }
      if(pwr_state == 3U){
        rele_inv(&pwr_state);
      }
      buZZ(25U<<pwr_state);
      time(&strt_lohi);
    }
    if(keys == PLUS_NUM){
      time(&strt_pls);
    }
    if(keys == MINUS_NUM){
      time(&strt_mns);
    }
  }else{
    if(keys == POWER_SET_NUM){
      strt_pwr = 0U;
      if(pwr_state){
        buZZ(1000U); 
        pwr_state = 0U;
        rele_inv(&pwr_state);
      }else{
        buZZ(50U);
        pwr_state = 1U;
      }
    }
    if(keys == LO_HI_NUM){
      strt_lohi = 0U;
    }
    if(keys == PLUS_NUM){
      strt_pls = 0U;
    }
    if(keys == MINUS_NUM){
      strt_mns = 0U;
    }
  }
  CLIRE_BIT(flagi.tim_flg);
}
bool ICACHE_FLASH_ATTR timer_f(uint16_t t_ms)//аргумент время в мс
{
  os_timer_disarm(&timer_press);
  os_timer_setfn(&timer_press, (os_timer_func_t *)debonce_f, NULL);
  os_timer_arm(&timer_press, t_ms, 0);
  return true;
}
static void gpio_isr_handler(void *arg)
{
    if(!GET_BIT(flagi.tim_flg)){
      timer_f(DEBONCE) ? SET_BIT(flagi.tim_flg):CLIRE_BIT(flagi.tim_flg);
      keys = (uint32_t) arg;
    }
}
void ICACHE_FLASH_ATTR knopki_init()
{
  gpio_config_t pGPIOConfig = {
    .pin_bit_mask = POWER_SET | LO_HI | MINUS | PLUS,	// кнопка питания // кнопка LO_HI // кнопка MINUS // кнопка PLUS
    .intr_type = GPIO_INTR_ANYEDGE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .mode = GPIO_MODE_INPUT
  };
  gpio_config(&pGPIOConfig);

  //create a queue to handle gpio event from isr
  //gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
  //install gpio isr service
  gpio_install_isr_service(0);

  gpio_isr_handler_add(POWER_SET_NUM, gpio_isr_handler, (void *) POWER_SET_NUM);
  gpio_isr_handler_add(LO_HI_NUM, gpio_isr_handler, (void *) LO_HI_NUM);
  gpio_isr_handler_add(MINUS_NUM, gpio_isr_handler, (void *) MINUS_NUM);
  gpio_isr_handler_add(PLUS_NUM, gpio_isr_handler, (void *) PLUS_NUM);
}

static void ICACHE_FLASH_ATTR off_lo_hi(uint8_t *state)
{
  err = gpio_set_level(RELE_1, (1U & *state));
    if(err != ESP_OK){
        printf("rele1 err %s\n ", esp_err_to_name(err));
    }	  
  err = gpio_set_level(RELE_2, ((2U & *state)>>1));
    if(err != ESP_OK){
        printf("rele2 err %s\n ", esp_err_to_name(err));
    }
  if(*state){
    if(!GET_BIT(flagi.rel_on)){
      heat_time = time(NULL);
      SET_BIT(flagi.rel_on);
    }
  }else{
    CLIRE_BIT(flagi.forse_on);
    CLIRE_BIT(flagi.rel_on);
  } 
  //printf("rele: %d\n", *state);
}

void ICACHE_FLASH_ATTR start_init()
{
    out_init();
    buzz_pwm_init();
    SET_BIT(flagi.start_init_f);
}
void ICACHE_FLASH_ATTR buZZ(u16_t time_beep)
{
    if(!GET_BIT(flagi.start_init_f))
    {
        start_init();
    }else{
        err = pwm_start();
        if(err != ESP_OK){
            //printf("pwm start again %s\n ", esp_err_to_name(err));
        }	
    }
      os_timer_disarm(&timer_buzz);
      os_timer_setfn(&timer_buzz, (os_timer_func_t *)timer_func_user,NULL);
      os_timer_arm(&timer_buzz, time_beep, 0);
}


void ICACHE_FLASH_ATTR termosens_task(void *pvParameters)
{
	adc_config_t config ={
		.clk_div = 32,
		.mode = ADC_READ_TOUT_MODE
	};
	err = adc_init(&config);
	if(err != ESP_OK){
		printf("ацп_init_err: %d\n",err);
	}
  while (1) 
  {
    if(!GET_BIT(flagi.mess_on)){
      SET_BIT(flagi.mess_on);
      izmer();

      os_timer_disarm(&power_termsns);
      os_timer_setfn(&power_termsns, (os_timer_func_t *)adc_timer, NULL);
      os_timer_arm(&power_termsns, 100, 0);
    }else{
      vTaskDelay(2000/portTICK_RATE_MS);
    }	
  }
	vTaskDelete(NULL);
}
static uint8_t *auto_pwr()
{
  
  if((delta > 5U) || GET_BIT(flagi.forse_on)){
    fp = 3U;
    if(!GET_BIT(flagi.forse_on)) {SET_BIT(flagi.forse_on);}
  }
  if((delta < 5U) && !GET_BIT(flagi.forse_on)){
    current_time = time(NULL);
    rele_inv(&fp);
    if(fp != invers){
      fp = invers;
    }else if((current_time - heat_time) > 3600U){
      SET_BIT(flagi.forse_on);
    }
    

  }
  return &fp;
}
bool delete_broker()
{
  current_time = time(NULL);
  if(!strt_lohi || ((current_time - strt_lohi) < 5U)){
    return false;
  }
  buZZ(500U);
  return true;
}
bool delete_ap()
{
  current_time = time(NULL);
  if(!strt_pwr || ((current_time - strt_pwr) < 5U)){
    return false;
  }
  buZZ(500U);
  return true;
}
static uint8_t *termo_set()
{
  if(strt_mns && (celevaya> 5U)){
    --celevaya;
    hysteresis(false);
    buZZ(50U);
  }
  if(strt_pls && (celevaya < 30U)){
    ++celevaya;
    hysteresis(false);
    buZZ(50U);
  }
  return &celevaya;
}
uint8_t *mem_state_rw(uint8_t *mem)
{
  if(mem != NULL){
    state_mem = *mem;
  }
  return &state_mem;
}
static void heater()
{
  if(delta != 0U){
    if(pwr_state == 0U || pwr_state == 3U){
      off_lo_hi(&pwr_state);
    }else if(pwr_state == 1U){
      off_lo_hi(auto_pwr());
    }else if(pwr_state == 2U){
      off_lo_hi(&invers);
    }
  }else{
    off_lo_hi(&delta);
  }
}
char *state_control(uint8_t *state)
{
  if(state != NULL){
    celevaya = *state >> 2U;
    pwr_state = *state & 3U;
    rele_inv(&pwr_state);
  }
  return bufer;
}
void ICACHE_FLASH_ATTR termostat_task(void *pvParameters)
{
  celevaya = state_mem >> 2U;
  pwr_state = state_mem & 3U;
	while (1) 
		{
      celevaya = *termo_set();
      if(state_mem != (celevaya << 2U) + pwr_state){
        state_mem = (celevaya << 2U) + pwr_state;
        heater();
        //printf("delta %d celevaya %d pwrstate %d statemem %d curenttime %li \n", delta, celevaya, pwr_state, state_mem, current_time);
      }
      if((kT != 0U) && kT < (KELVIN_0 + 1U)){
        if(delta != *delta_t()){
          delta = *delta_t();
          off_lo_hi(auto_pwr());
        }
      }
      if(delta != *delta_t()){
        delta = *delta_t();
        if(delta == 0U){
          rele_inv(&delta);
        }
        heater();
        
        //printf("delta %d delta_t %d celevaya %d pwrstate %d statemem %d curenttime %li \n", delta, *delta_t(), celevaya, pwr_state, state_mem, current_time);
      }
		  vTaskDelay(200/portTICK_RATE_MS);	
		}
	vTaskDelete(NULL);
}