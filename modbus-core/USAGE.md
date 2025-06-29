# Modbus Core 模块使用说明

[...保持现有内容不变...]

## 更多实际应用场景

### 场景1：批量读取设备状态寄存器
```c
// 读取PLC设备状态(地址3，寄存器40050-40059共10个寄存器)
uint8_t plc_request[] = {0x03, 0x03, 0x00, 0x31, 0x00, 0x0A};
modbus_t plc_ctx;
if (modbus_parse_request(&plc_ctx, plc_request, sizeof(plc_request)) == 0) {
    uint8_t plc_response[256];
    uint16_t plc_registers[10] = {
        0x0001, // 设备状态
        0x1388, // 运行小时数(5000小时)
        0x0064, // 当前负载(100%)
        0x00C8, // 温度(200°C)
        // 其他状态寄存器...
    };
    
    // 构建响应
    int len = modbus_build_response(&plc_ctx, plc_response, sizeof(plc_response));
    
    // 解析设备状态
    uint16_t status = plc_registers[0];
    float runtime_hours = plc_registers[1] / 1.0f;
    float load_percent = plc_registers[2] / 1.0f;
    float temperature = plc_registers[3] / 1.0f;
}
```

### 场景2：批量设置设备开关状态
```c
// 设置8台电机开关状态(地址4，线圈20001-20008)
uint8_t motor_request[] = {0x04, 0x0F, 0x00, 0x00, 0x00, 0x08, 0x01, 0x55}; 
// 0x55 = 01010101 (交替开启电机)
modbus_t motor_ctx;
if (modbus_parse_request(&motor_ctx, motor_request, sizeof(motor_request)) == 0) {
    uint8_t motor_response[256];
    
    // 构建响应
    int len = modbus_build_response(&motor_ctx, motor_response, sizeof(motor_response));
    
    // 解析设置的位状态
    uint8_t motor_bits[8];
    modbus_set_bits_from_byte(motor_bits, 0, 0x55);
    // motor_bits = {0,1,0,1,0,1,0,1}
}
```

### 场景3：处理异常响应
```c
// 尝试读取不存在的寄存器(地址5，寄存器50000)
uint8_t err_request[] = {0x05, 0x03, 0xC3, 0x50, 0x00, 0x01};
modbus_t err_ctx;
if (modbus_parse_request(&err_ctx, err_request, sizeof(err_request)) == 0) {
    uint8_t err_response[256];
    
    // 构建错误响应(非法数据地址错误)
    err_response[0] = 0x05; // 设备地址
    err_response[1] = 0x83; // 错误功能码(0x03 + 0x80)
    err_response[2] = 0x02; // 异常代码(02=非法数据地址)
    
    // 模拟错误处理
    if (err_response[1] & 0x80) {
        printf("Modbus错误: 异常代码 %d\n", err_response[2]);
    }
}
```

### 场景4：数据采集与持久化
```c
// 采集数据并保存到CSV文件
void save_sensor_data(const char* filename) {
    FILE* fp = fopen(filename, "a");
    if (!fp) return;
    
    // 1. 读取传感器数据
    uint8_t request[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x02};
    modbus_t ctx;
    if (modbus_parse_request(&ctx, request, sizeof(request)) == 0) {
        uint8_t response[256];
        uint16_t registers[2] = {0x4220, 0x51EB}; // 示例数据
        
        // 2. 构建响应
        modbus_build_response(&ctx, response, sizeof(response));
        
        // 3. 解析数据
        float temp = modbus_get_float_abcd(registers);
        float humidity = modbus_get_float_abcd(&registers[2]);
        
        // 4. 保存到CSV
        time_t now = time(NULL);
        fprintf(fp, "%lld,%.2f,%.2f\n", (long long)now, temp, humidity);
    }
    
    fclose(fp);
}
```

### 场景5：混合读写操作
```c
// 先读取当前值，然后写入新配置
void update_config(uint8_t device_id) {
    // 1. 读取当前配置
    uint8_t read_req[] = {device_id, 0x03, 0x00, 0x10, 0x00, 0x02};
    modbus_t read_ctx;
    if (modbus_parse_request(&read_ctx, read_req, sizeof(read_req)) == 0) {
        uint8_t read_res[256];
        uint16_t current_config[2];
        
        // 模拟当前配置值
        current_config[0] = 0x1234;
        current_config[1] = 0x5678;
        
        // 2. 构建读取响应
        modbus_build_response(&read_ctx, read_res, sizeof(read_res));
        
        // 3. 准备新配置(基于当前值)
        uint16_t new_config[2];
        modbus_set_float_abcd(
            modbus_get_float_abcd(current_config) * 1.1f, // 增加10%
            new_config
        );
        
        // 4. 写入新配置
        uint8_t write_req[] = {
            device_id, 0x10, 0x00, 0x10, 0x00, 0x02, 0x04,
            (uint8_t)(new_config[0] >> 8), (uint8_t)new_config[0],
            (uint8_t)(new_config[1] >> 8), (uint8_t)new_config[1]
        };
        
        modbus_t write_ctx;
        if (modbus_parse_request(&write_ctx, write_req, sizeof(write_req)) == 0) {
            uint8_t write_res[256];
            modbus_build_response(&write_ctx, write_res, sizeof(write_res));
        }
    }
}
```