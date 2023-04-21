#include <openssl/sha.h>
//------------------------------------------------------------------------------

#include "sha256.h"
//------------------------------------------------------------------------------

SHA256::SHA256()
{
  evpCtx = EVP_MD_CTX_new();

  Init();
}
//------------------------------------------------------------------------------

SHA256::~SHA256()
{
  EVP_MD_CTX_free(evpCtx);
}
//------------------------------------------------------------------------------

void SHA256::Init()
{
  EVP_DigestInit_ex(evpCtx, EVP_sha256 (), NULL);

  Finalized = false;
}
//------------------------------------------------------------------------------

void SHA256::Update(const unsigned char *aMessage, uint aLen)
{
  if (Finalized)
    Init();

  EVP_DigestUpdate (evpCtx, aMessage, aLen);
}
//------------------------------------------------------------------------------

void SHA256::Final()
{
  if (Finalized)
    return;

  unsigned int Len = 0;
  unsigned char Sha256 [EVP_MAX_MD_SIZE] = {0};
  EVP_DigestFinal_ex(evpCtx, Sha256, &Len);

  // Convert to hex
  const char Map[] = "0123456789abcdef";
  char Buff[EVP_MAX_MD_SIZE * 2 + 1];
  char* SPtr = (char*)Sha256;
  char* DPtr = Buff;
  for (size_t T = 0; T < Len; T++)
  {
    *DPtr++ = Map[(*SPtr >> 4) & 0xf];
    *DPtr++ = Map[(*SPtr++) & 0xf];
  }
  *DPtr = 0;
  Sha256Sum = std::string(Buff);

  Finalized = true;
}
//------------------------------------------------------------------------------

std::string SHA256::GetChecksum()
{
  Final();

  return Sha256Sum;
}
//------------------------------------------------------------------------------
