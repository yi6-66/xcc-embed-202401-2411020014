/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "gpio.h"
#include "fsmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdint.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */


/* USER CODE BEGIN 0 */

// 第一关：地址映射
#define PORT16(addr) (*((volatile uint16_t *)(addr)))
#define SEG_ADDR    0x68000000
#define LCD_CMD_ADDR  0x6C000000
#define LCD_DATA_ADDR (0x6C000000 | (1 << 7))

void cmd(uint8_t c) { PORT16(LCD_CMD_ADDR) = c; }
void data(uint16_t d) { PORT16(LCD_DATA_ADDR) = d; }

// 第二关：数码管驱动
const uint8_t seg_table[11] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90, 0xFF}; // 多加了0xFF（索引10）表示空白

void scan_frame(const uint8_t digit[6]) {
    uint16_t select = 0xFE; // 1111 1110
    for(int i = 0; i < 6; i++) {
        PORT16(SEG_ADDR) = (select << 8) | seg_table[digit[i]];
        select = (select << 1) | 0x01; // 左移，如 1111 1101
        HAL_Delay(1); // 短暂延时防重影
    }
}

// 第四关：LCD初始化和刷屏
void panel_init(void) {
    // 若复位引脚受MCU控制，这里需要拉低再拉高复位，若直接接RST则直接延时
    HAL_Delay(50); 
    cmd(0xCF); data(0x00); data(0x83); data(0X30);
    cmd(0xED); data(0x64); data(0x03); data(0X12); data(0x81);
    cmd(0xE8); data(0x85); data(0x01); data(0x79);
    cmd(0xCB); data(0x39); data(0x2C); data(0x00); data(0x34); data(0x02);
    cmd(0xF7); data(0x20);
    cmd(0xEA); data(0x00); data(0x00);
    cmd(0xC0); data(0x1B);
    cmd(0xC1); data(0x12);
    cmd(0xC5); data(0x08); data(0x26);
    cmd(0xC7); data(0xB0);
    
    // 关键：0x48 横屏，0x3A 设置RGB565
    cmd(0x36); data(0x48); 
    cmd(0x3A); data(0x55); 
    
    cmd(0xB1); data(0x00); data(0x1B);
    cmd(0xB6); data(0x0A); data(0xA2);
    cmd(0xF2); data(0x00); 
    cmd(0x26); data(0x01); 
    cmd(0xE0); data(0x0F); data(0x31); data(0x2B); data(0x0C); data(0x0E); data(0x08); data(0x4E); data(0xF1); data(0x37); data(0x07); data(0x10); data(0x03); data(0x0E); data(0x09); data(0x00);
    cmd(0xE1); data(0x00); data(0x0E); data(0x14); data(0x03); data(0x11); data(0x07); data(0x31); data(0xC1); data(0x48); data(0x08); data(0x0F); data(0x0C); data(0x31); data(0x36); data(0x0F);
    cmd(0x11); // 退出睡眠
    HAL_Delay(120);
    cmd(0x29); // 开显示
}

void window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    cmd(0x2A); data(x0 >> 8); data(x0 & 0xFF); data(x1 >> 8); data(x1 & 0xFF);
    cmd(0x2B); data(y0 >> 8); data(y0 & 0xFF); data(y1 >> 8); data(y1 & 0xFF);
    cmd(0x2C);
}

void fill(uint16_t color) {
    for(uint32_t i = 0; i < 76800; i++) { // 320*240
        data(color);
    }
}

// 第五关：字模显示
// 5列x7行的字模
const uint8_t font_H[7] = {0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}; 
const uint8_t font_I[7] = {0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E};
const uint8_t font_ex[7] = {0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x04}; // 感叹号

void glyph(uint16_t x, uint16_t y, const uint8_t rows[7]) {
    window(x, y, x + 11, y + 13); // 5列放大2倍=10，加1列间距=12；7行放大2倍=14，所以高度+13
    
    for(int py = 0; py < 14; py++) {
        for(int px = 0; px < 12; px++) {
            int row = py / 2;
            int col = px / 2;
            
            // 如果是前5列（即字符本身），且对应位为1，则写黑字
            if (col < 5 && (rows[row] & (0x10 >> col))) { 
                data(0x0000); // 黑字
            } else {
                data(0xFFFF); // 白底
            }
        }
    }
}
/* USER CODE END 0 */
/* USER CODE END 0 */
/* USER CODE END 0 */
// LCD2 同理
/* USER CODE END 0 */
void SystemClock_Config(void);
/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_FSMC_Init();
  /* USER CODE BEGIN 2 */


  /* USER CODE BEGIN 2 */
  // 测试数码管
  //uint8_t digits[6] = {1, 2, 3, 4, 5, 6};
  
  // LCD初始化测试
  panel_init();
  
  // 画色块测试
  window(0, 0, 479, 319); // 全屏窗口
  fill(0xF800); // 红色 (RGB565)
  HAL_Delay(500);
  fill(0x07E0); // 绿色
  HAL_Delay(500);
  fill(0x001F); // 蓝色
  HAL_Delay(500);
  fill(0xFFFF); // 白色背景

	  // 4. 画字 "HI!"
  glyph(142, 113, font_H);
  glyph(154, 113, font_I);
  glyph(166, 113, font_ex);
  
  // 5. 准备数码管滚动数据 (讲义要求：五个前导空白、1~8、五个后缀空白)
  // 使用 10 代表空白
  uint8_t scroll_data[18] = {10,10,10,10,10,0,1,2,3,4,5,10,10,10,10,10,10,10};
  uint8_t digits[6] = {10,10,10,10,10,10};
  uint32_t last_tick = 0;
  uint8_t start_idx = 0;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    // 高频扫描数码管，防止闪烁
    scan_frame(digits); 
    
    // 滚动逻辑：每 250ms 截取数组的一段放入 digits 并更新显示
    if (HAL_GetTick() - last_tick >= 250) {
        last_tick = HAL_GetTick();
        
        // 截取 6 个数
        for(int i = 0; i < 6; i++) {
            digits[i] = scroll_data[start_idx + i];
        }
        
        start_idx++;
        if(start_idx > 12) { // 数组长18，截取长6，最大索引不能超过 18-6=12
            start_idx = 0; // 回到开头
        }
    }
    /* USER CODE END WHILE */
  }
  /* USER CODE END 2 */




  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	
  while (1)
  {
    /* USER CODE END WHILE */
	
    /* USER CODE BEGIN 3 */
		    /* USER CODE BEGIN 3 */
    // 主循环里不断扫描数码管，防止闪烁
    scan_frame(digits);
    
    // 显示一次 HI! 即可，不用放在死循环里狂刷
    // 静态变量控制只画一次
    static uint8_t drawn = 0;
    if(!drawn) {
        glyph(100, 100, font_H);
        glyph(120, 100, font_I);
        glyph(140, 100, font_ex);
        drawn = 1;
    }
    
    // 如果你想数码管跑起来，可以在这里加计数逻辑
    // ...
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
