#ifndef CRYPTO_DEVS_HPP
#define CRYPTO_DEVS_HPP

#include <cryptlib.h>
#include <rsa.h>
#include <sha.h>
#include <osrng.h>
#include <hex.h>
#include <base64.h>
#include <iostream>
#include <fstream>
#include <string>

using namespace CryptoPP;

class Crypto {
public:
    // Function to convert RSA Private Key to Base64-encoded string
    static std::string PrivateKeyToBase64(const RSA::PrivateKey& privateKey);

    // Function to convert RSA Public Key to Base64-encoded string
    static std::string PublicKeyToBase64(const RSA::PublicKey& publicKey);

    // Function to create a new RSA private key
    static RSA::PrivateKey GeneratePrivateKey();

    // Function to create RSA public key from private key
    static RSA::PublicKey GeneratePublicKey(const RSA::PrivateKey& privateKey);

    // Function to decode a Base64 string into an RSA private key
    static RSA::PrivateKey LoadPrivateKeyFromBase64(const std::string& base64PrivateKey);

    // Function to decode a Base64 string into an RSA public key
    static RSA::PublicKey LoadPublicKeyFromBase64(const std::string& base64PublicKey);

    // Function to hash data (SHA-256)
    static void HashData(const std::string& data, byte* hash);

    // Function to sign the hash using RSA private key and return the signature as a Base64 string
    static std::string SignData(const std::string& data, const std::string& base64PrivateKey);

    // Function to verify the signature using RSA public key and Base64-encoded signature string
    static bool VerifySignature(const std::string& data, const std::string& base64PublicKey, const std::string& base64Signature);
};

#endif // CRYPTO_DEVS_HPP
