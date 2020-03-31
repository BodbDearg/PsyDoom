#pragma once

struct master_status_structure;

bool wess_dig_lcd_loader_init(master_status_structure* const pMStat) noexcept;
void wess_dig_set_sample_position() noexcept;
void wess_dig_lcd_data_read() noexcept;
void wess_dig_lcd_psxcd_sync() noexcept;
void wess_dig_lcd_load() noexcept;
