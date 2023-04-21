#ifndef __SHA256_H__
#define __SHA256_H__
//------------------------------------------------------------------------------

#include <string>
#include <openssl/evp.h>
//------------------------------------------------------------------------------

class SHA256
{
  private:
    bool Finalized;

    EVP_MD_CTX* evpCtx;

    void Final();

    std::string Sha256Sum;

  public:
    SHA256();
    virtual ~SHA256();

    void Init();
    void Update(const unsigned char *aMessage, uint aLen);

    std::string GetChecksum();
};
//------------------------------------------------------------------------------

#endif  //__SHA256_H__
