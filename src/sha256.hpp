#include <string>

class HashFunction {
    protected:
        typedef unsigned char register8;
        typedef unsigned int  register32;
        typedef unsigned long long register64;

        const static register32 hashKeys[];
        static const unsigned int BLOCK_SIZE_256 = (512/8);
    
    public:
        void stateRegister();
        void adjustDigest(const unsigned char * text, unsigned int textLength); //process data in chunks
        void digestFinal(unsigned char * digest);
        static const unsigned int PADD_SIZE = (256/8); //represents 32 bytes of opt

    protected:
        void compress(const unsigned char* message, unsigned int blockNB);
        unsigned int srTotalLength;
        unsigned int srLength;
        unsigned char srBlock[2*BLOCK_SIZE_256];
        register32 sr[8];
};

std::string sha256(std::string input);
