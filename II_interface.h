int SetUpDevice (const char* device);
void writeRegister (int ttyDesc, unsigned int address, unsigned int data);
void readRegister (int ttyDesc, unsigned int address, unsigned int* data);
