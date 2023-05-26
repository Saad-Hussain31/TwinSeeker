#include "sha256.hpp"
#include <cstring>

#define SHAF_UNPACK32(x, str)                 \
{                                             \
    *((str) + 3) = (register8) ((x)      );       \
    *((str) + 2) = (register8) ((x) >>  8);       \
    *((str) + 1) = (register8) ((x) >> 16);       \
    *((str) + 0) = (register8) ((x) >> 24);       \
}

const unsigned int HashFunction::hashKeys[64] = 
            {0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
             0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
             0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
             0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
             0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
             0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
             0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
             0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
             0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
             0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
             0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
             0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
             0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
             0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
             0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
             0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2} ;


void HashFunction::stateRegister() {
    sr[0] = 0x6a09e667;
    sr[1] = 0xbb67ae85;
    sr[2] = 0x3c6ef372;
    sr[3] = 0xa54ff53a;
    sr[4] = 0x510e527f;
    sr[5] = 0x9b05688c;
    sr[6] = 0x1f83d9ab;
    sr[7] = 0x5be0cd19;
    srLength = 0;
    srTotalLength = 0;
}

void HashFunction::adjustDigest(const unsigned char* text,
                                unsigned int textLength) {
                                    
    unsigned int blockNB;
    unsigned int key, newLen, remLen, tempLen;
    const unsigned char* shiftedMsg;

    tempLen = BLOCK_SIZE_256 - srLength;
    remLen = textLength < tempLen ? textLength : tempLen;
    memcpy(&srBlock[srLength], text, remLen);
    if(srLength + remLen < BLOCK_SIZE_256) {
        srLength += tempLen;
        return;
    }

    newLen = textLength - remLen;
    blockNB = newLen / BLOCK_SIZE_256;
    shiftedMsg = text + remLen;
    compress(shiftedMsg, blockNB);
    remLen = newLen % BLOCK_SIZE_256;
    memcpy(srBlock, &shiftedMsg[blockNB << 6], remLen);
    srLength = remLen;
    srTotalLength += (blockNB +1) << 6;
}

void HashFunction::digestFinal(unsigned char *digest)
{
    unsigned int blockNB;
    unsigned int pmLength;
    unsigned int length;
    int i;
    blockNB = (1 + ((BLOCK_SIZE_256 - 9)
                     < (srLength % BLOCK_SIZE_256)));
    length = (srTotalLength + srLength) << 3;
    pmLength = blockNB << 6;
    memset(srBlock + srLength, 0, pmLength - srLength);
    srBlock[srLength] = 0x80;
    SHAF_UNPACK32(length, srBlock + pmLength - 4);
    compress(srBlock, blockNB);
    for (i = 0 ; i < 8; i++) {
        SHAF_UNPACK32(sr[i], &digest[i << 2]);
    }
}



