// set start and stop address
    // command 'set column address' (y-axis)
    GPIO_PORTM_DATA_R = 0x2A;           // Write command byte
    // Set start column address high byte
    GPIO_PORTM_DATA_R = (start.y >> 8); // Write data byte
    // Set start column address low byte
    GPIO_PORTM_DATA_R = start.y;        // Write data byte
    // Set stop column address high byte
    GPIO_PORTM_DATA_R = stop.y >> 8;    // Write data byte
    // Set stop column address low byte
    GPIO_PORTM_DATA_R = stop.y;         // Write data byte
    // command 'set page address' (x-axis)
    GPIO_PORTM_DATA_R = 0x2B;           // Write command byte
    // Set start page address high byte
    GPIO_PORTM_DATA_R = start.x >> 8;   // Write data byte
    // Set start page address low byte
    GPIO_PORTM_DATA_R = start.x;        // Write data byte
    // Set stop page address high byte
    GPIO_PORTM_DATA_R = stop.x >> 8;    // Write data byte
    // Set stop page address low byte
    GPIO_PORTM_DATA_R = stop.x;         // Write data byte
// command 'start writing'
    GPIO_PORTM_DATA_R = 0x2c;           // Write command byte

// Send pixel data RGB
    GPIO_PORTM_DATA_R = color.red;      // Write data byte
    GPIO_PORTM_DATA_R = color.green;    // Write data byte
    GPIO_PORTM_DATA_R = color.blue;     // Write data byte
