# isv

**isv** is utility for controlling Voltronic hybrid solar inverters that use P18 protocol. **isv** has full P18
support with all known methods implemented. It was written for use with InfiniSolar V 5kW  inverter and it's the only
inverter it has been tested with so far, but it should work with other inverters using P18 protocol as well. Adding
support for other protocols (such as P16 or P17) by splitting them into separate modules is possible in future.

For now, only USB connection is supported and tested (I just don't have the RS-232 cable with this weird RJ-style plug
lol), but RS-232 support will be added eventually.

It's written in pure C99 with almost zero dependencies. It uses [libvoltronic](https://github.com/jvandervyver/libvoltronic)
for underlying device interaction. but you don't need to download and build it separately as **isv** comes with its own
slightly modified libvoltronic version.

It can output data in different formats (human-readable tables, conveniently-parsable tables and even JSON) so you can
easily integrate it in your project. 

For now only Linux and macOS are supported and tested. Other operating systems will be supported later.

## Requirements

- `pkg-config`
- `hidapi`
  - On Linux, you should be able to install it from your distro's package manager
  - On macOS, `brew install hidapi`   
  
## Building

Just run `make`. If you want to install it, `make install` will do the job.

## Usage

Run `isv` without arguments to see the full options list. For the sake of good readmes it's also written here.

### Common options

- **`-r`** `COMMAND`<br>
  **`--raw`** `COMMAND` - execute arbitrary command and print inverter's response.<br>
    Command example: `^P005PI`
    
- **`-t `** `TIMEOUT`<br>
  **`--timeout`** `TIMEOUT` - device read timeout, in milliseconds.<br>Example: `-t 5000`
  
- **`-v`**, **`--verbose`** - print debug information, like hexdumps of communication traffic with inverter

- **`-p`**, **`--pretend`** - do not actually execute anything on inverter, but output some debug info. Little use for
  normal people. Doesn't work with `--raw`.
  
- **`-f`** `FORMAT`<br>
  **`--format`** `FORMAT` - output format for `--get-*` and `--set-*` options, you can find list of supported 
  formats below.
  
### Get options

- **`--get-protocol-id`** - returns protocol id. Should be always `18` as it's the only one supported.

- **`--get-date-time`** - returns date and time from inverter

- **`--get-total-generated`** - returns total generated energy, in kWatts (or Watts?). The documentation says it should be 
  kilowatts, but my inverter says that we generated almost 200,000 for the last two months... It's must be Watts. My
  guess is that it reports Watts first and when it reaches some kind of integer limit it switches to kWatts.  

- **`--get-year-generated`** `YYYY` - returns generated energy for specified year, in kWatts (or Watts? see above)

- **`--get-month-generated`** `YYYY` `MM` - returns generated energy for specified month, in kWatts (or Watts? see above)

- **`--get-day-generated`** `YYYY` `MM` `DD` - returns generated energy for specified day, in Watts.

- **`--get-series-number`** - returns series number. Or maybe serial number. The documentation is written by Chinese in
  bad english.

- **`--get-cpu-version`** - returns main and slave CPU versions.

- **`--get-rated-information`** - returns rated information.

- **`--get-general-status`** - returns general status, many cool stuff here. Usually this is what you want to read.

- **`--get-working-mode`** - returns working mode.

- **`--get-faults-warnings`** - returns fault and warning status.

- **`--get-flags`** - returns state of a set of flags, or toggles, like backlight or buzzer ON or OFF, etc.

- **`--get-defaults`** - returns default values of some changeable parameters and default flags values.

- **`--get-max-charging-current-selectable-values`**

- **`--get-max-ac-charging-current-selectable-values`**

- **`--get-parallel-rated-information`** `ID`<br>
  `ID` - parallel machine ID

- **`--get-parallel-general-status`** `ID`<br>
  `ID` - parallel machine ID

- **`--get-ac-charge-time-bucket`**

- **`--get-ac-supply-load-time-bucket`**

### Set options

- **`--set-loads-supply`** `0|1`

- **`--set-flag`** `FLAG` `0|1`

  List of flags:
  
  - `BUZZ` - Silence buzzer or open buzzer
  - `OLBP` - Overload bypass function
  - `LCDE` - LCD display escape to default page after 1min timeout
  - `OLRS` - Overload restart
  - `OTRS` - Overload temperature restart
  - `BLON` - Backlight on
  - `ALRM` - Alarm on primary source interrupt
  - `FTCR` - Fault code record
  - `MTYP` - Machine type (1=Grid-Tie, 0=Off-Grid-Tie)

- **`--set-defaults`**<br>
  Reset changeable parameters to their default values.

- **`--set-battery-max-charging-current`** `ID` `AMPS`<br>
  `ID` - parallel machine ID (use 0 for a single model)<br>
  `AMPS` - use `--get-max-charging-current-selectable-values` to see a list of allowed currents
  
- **`--set-battery-max-ac-charging-current`** `ID` `AMPS`<br>
  `ID` - parallel machine ID (use 0 for a single model)<br>
  `AMPS` - use `--get-max-ac-charging-current-selectable-values` to see a list of allowed currents
  
- **`--set-ac-output-freq`** `50|60`

- **`--set-battery-max-charging-voltage`** `CV` `FV`<br>
  `CV` - constant voltage (48.0 ~ 58.4)<br>
  `FV` - float voltage (48.0 ~ 58.4)
  
- **`--set-ac-output-rated-voltage`** `V`<br>
  `V` - voltage. Allowed voltages are `202`, `208`, `220`, `230` and `240`
  
- **`--set-output-source-priority`** `PRIORITY`

  List of priorities:
   
  - `SUB` is for *Solar-Utility-Battery*<br>
  - `SBU` is for *Solar-Battery-Utility*

- **`--set-battery-charging-thresholds`** `CV` `DV`<br>
  Sets battery re-charging and re-discharigng voltages when utility is available.
  
  `CV` - re-charging voltage<br>
   *For 12V unit:* `11`, `11.3`, `11.5`, `11.8`, `12`, `12.3`, `12.5` or `12.8`<br>
   *For 24V unit:* `22`, `22.5`, `23`, `23.5`, `24`, `24.5`, `25` or `25.5`<br>
   *For 48V unit:* `44`, `45`, `46`, `47`, `48`, `49`, `50` or `51`
   
   `DV` - re-discharging voltage<br>
   *For 12V unit:* `0`, `12`, `12.3`, `12.5`, `12.8`, `13`, `13.3`, `13.5`, `13.8`, `14`, `14.3` or `14.5`<br>
   *For 24V unit:* `0`, `24`, `24.5`, `25`, `25.5`, `26`, `26.5`, `27`, `27.5`, `28`, `28.5` or `29`<br>
   *For 48V unit:* `0`, `48`, `49`, `50`, `51`, `52`, `53`, `54`, `55`, `56`, `57` or `58`<br>

- **`--set-charging-source-priority`** `ID` `PRIORITY`<br>
  `ID` - parallel machine ID (use 0 for a single model).
  
  List of priorities: 
  
  - `SF` for *Solar-First*<br>
  - `SU` for *Solar-and-Utility*<br>
  - `S` for *Solar-Only*

- **`--set-solar-power-priority`** `PRIORITY`

  List of priorities:
  
  - `BLU` for *Battery-Load-Utility*<br>
  - `LBU` for *Load-Battery-Utility*

- **`--set-ac-input-voltage-range`** `RANGE`

  List of ranges:
  
  - `APPLIANCE`
  - `UPS`

- **`--set-battery-type`** `AGM|FLOODED|USER`

- **`--set-output-model`** `ID` `MODEL`<br>
  `ID` - parallel machine ID (use 0 for a single model).
  
  List of allowed models:
  
  - `SM` - Single module
  - `P` - Parallel output
  - `P1` - Phase 1 of three phase output
  - `P2` - Phase 2 of three phase output
  - `P3` - Phase 3 of three phase

- **`--set-battery-cutoff-voltage`** `V`<br>
  `V` - cut-off voltage (40.0 ~ 48.0)

- **`--set-solar-configuration`** `ID`<br>
  `ID` - serial number

- **`--clear-generated-data`**<br>
  Clears all data of generated energy.

- **`--set-date-time`** `YYYY` `MM` `DD` `hh` `mm` `ss`<br>
  `YYYY` - year<br>
  `MM` - month<br>
  `DD` - day<br>
  `hh` - hours<br>
  `mm` - minutes<br>
  `ss` - seconds

- **`--set-ac-charge-time-bucket`** `START` `END`<br>
  `START` - starting time, `hh:mm` format<br>
  `END` - ending time, `hh:mm` format

- **`--set-ac-supply-load-time-bucket`** `START` `END`<br>
  `START` - starting time, `hh:mm` format<br>
  `END` - ending time, `hh:mm` format

### Formats
- `table` - human-readable table. This is used by default.

  Output example:
  ```
  Grid voltage:                      0.0 V
  Grid frequency:                    0.0 Hz
  AC output voltage:                 230.1 V
  AC output frequency:               50.0 Hz
  AC output apparent power:          114 VA
  AC output active power:            69 Wh
  Output load percent:               2%
  Battery voltage:                   49.5 V
  Battery voltage from SCC:          0.0 V
  Battery voltage from SCC2:         0.0 V
  Battery discharge current:         1 A
  Battery charging current:          0 A
  Battery capacity:                  73%
  Inverter heat sink temperature:    32 °C
  MPPT1 charger temperature:         0 °C
  MPPT2 charger temperature:         0 °C
  PV1 Input power:                   0.00 Wh
  PV2 Input power:                   0.00 Wh
  PV1 Input voltage:                 0.0 V
  PV2 Input voltage:                 0.0 V
  Setting value configuration state: Something changed
  MPPT1 charger status:              Abnormal
  MPPT2 charger status:              Abnormal
  Load connection:                   Connected
  Battery power direction:           Discharge
  DC/AC power direction:             DC/AC
  Line power direction:              Do nothing
  Local parallel ID:                 0
  ```
  
- `parsable-table`

  Output example:
  
  ```
  grid_voltage 0.0 V
  grid_freq 0.0 Hz
  ac_output_voltage 230.0 V
  ac_output_freq 50.0 Hz
  ac_output_apparent_power 92 VA
  ac_output_active_power 52 Wh
  output_load_percent 1 %
  battery_voltage 49.5 V
  battery_voltage_scc 0.0 V
  battery_voltage_scc2 0.0 V
  battery_discharge_current 1 A
  battery_charging_current 0 A
  battery_capacity 73 %
  inverter_heat_sink_temp 32 °C
  mppt1_charger_temp 0 °C
  mppt2_charger_temp 0 °C
  pv1_input_power 0.00 Wh
  pv2_input_power 0.00 Wh
  pv1_input_voltage 0.0 V
  pv2_input_voltage 0.0 V
  settings_values_changed "Something changed"
  mppt1_charger_status Abnormal
  mppt2_charger_status Abnormal
  load_connected Connected
  battery_power_direction Discharge
  dc_ac_power_direction DC/AC
  line_power_direction "Do nothing"
  local_parallel_id 0
  ```
  
- `json` - JSON.

  Output example:
  
  ```
  {"grid_voltage":0.00,"grid_freq":0.00,"ac_output_voltage":229.90,"ac_output_freq":49.90,"ac_output_apparent_power":91,"ac_output_active_power":47,"output_load_percent":1,"battery_voltage":49.50,"battery_voltage_scc":0.00,"battery_voltage_scc2":0.00,"battery_discharge_current":1,"battery_charging_current":0,"battery_capacity":73,"inverter_heat_sink_temp":32,"mppt1_charger_temp":0,"mppt2_charger_temp":0,"pv1_input_power":0.00,"pv2_input_power":0.00,"pv1_input_voltage":0.00,"pv2_input_voltage":0.00,"settings_values_changed":"Something changed","mppt1_charger_status":"Abnormal","mppt2_charger_status":"Abnormal","load_connected":"Connected","battery_power_direction":"Discharge","dc_ac_power_direction":"DC/AC","line_power_direction":"Do nothing","local_parallel_id":0}
  ```
  
- `json-w-units` - JSON with units.

  Output example:
  
  ```
  {"grid_voltage":[0.00,"V"],"grid_freq":[0.00,"Hz"],"ac_output_voltage":[230.10,"V"],"ac_output_freq":[50.00,"Hz"],"ac_output_apparent_power":[92,"VA"],"ac_output_active_power":[53,"Wh"],"output_load_percent":[1,"%"],"battery_voltage":[49.50,"V"],"battery_voltage_scc":[0.00,"V"],"battery_voltage_scc2":[0.00,"V"],"battery_discharge_current":[1,"A"],"battery_charging_current":[0,"A"],"battery_capacity":[73,"%"],"inverter_heat_sink_temp":[32,"°C"],"mppt1_charger_temp":[0,"°C"],"mppt2_charger_temp":[0,"°C"],"pv1_input_power":[0.00,"Wh"],"pv2_input_power":[0.00,"Wh"],"pv1_input_voltage":[0.00,"V"],"pv2_input_voltage":[0.00,"V"],"settings_values_changed":"Something changed","mppt1_charger_status":"Abnormal","mppt2_charger_status":"Abnormal","load_connected":"Connected","battery_power_direction":"Discharge","dc_ac_power_direction":"DC/AC","line_power_direction":"Do nothing","local_parallel_id":0}
  ```

### Return codes

**isv** returns `0` on success, `1` on some input error (e.g. invalid argument) and `2` on communication failure (e.g.
failed to read response from inverter, or response is invalid).

## License

GPLv3