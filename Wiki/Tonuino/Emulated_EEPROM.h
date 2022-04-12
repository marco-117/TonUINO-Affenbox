// emulates EEPROM.put() .get() and .update() on LGT8F328P platform
// partially sourced from: https://playground.arduino.cc/Code/EEPROMWriteAnything/
template <class T> int EEPROM_put(int ee, const T& value)
{
  const byte* p = (const byte*)(const void*)&value;
  unsigned int i;

  for (i = 0; i < sizeof(value); i++) EEPROM.write(ee++, *p++);
  return i;
}

template <class T> int EEPROM_get(int ee, T& value)
{
  byte* p = (byte*)(void*)&value;
  unsigned int i;

  for (i = 0; i < sizeof(value); i++) *p++ = EEPROM.read(ee++);
  return i;
}

template <class T> void EEPROM_update(int ee, const T& value)
{
  EEPROM.read(ee) != value ? EEPROM.write(ee, value) : delay(0);
} 
