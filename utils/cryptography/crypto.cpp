#include "crypto.hpp"

// Function to convert RSA Private Key to Base64-encoded string
std::string Crypto::PrivateKeyToBase64(const RSA::PrivateKey& privateKey) {
    // Convert the private key to PEM format (binary)
    std::string pemData;
    StringSink ss(pemData);

    // Save the private key in PEM format
    privateKey.Save(ss);

    // Now Base64 encode the PEM format private key
    std::string base64Key;
    StringSource(pemData, true, 
        new Base64Encoder(new StringSink(base64Key), false)  // false to avoid line breaks in Base64 encoding
    );
    
    return base64Key;
}

// Function to convert RSA Public Key to Base64-encoded string (without PEM format)
std::string Crypto::PublicKeyToBase64(const RSA::PublicKey& publicKey) {
    // Step 1: Serialize the public key directly (using its raw data)
    std::string rawKeyData;
    StringSink rawKeySink(rawKeyData);  // Create a non-const StringSink to collect serialized data
    publicKey.Save(rawKeySink);  // Save the public key in its raw form (modulus + exponent)

    // Step 2: Base64 encode the raw key data
    std::string base64Key;
    StringSource(rawKeyData, true, new Base64Encoder(new StringSink(base64Key), false)); // false to avoid line breaks
    return base64Key;
}

// Function to create a new RSA private key
RSA::PrivateKey Crypto::GeneratePrivateKey() {
    AutoSeededRandomPool rng;
    RSA::PrivateKey privateKey;
    privateKey.GenerateRandomWithKeySize(rng, 2048);
    return privateKey;
}

// Function to create RSA public key from private key
RSA::PublicKey Crypto::GeneratePublicKey(const RSA::PrivateKey& privateKey) {
    return RSA::PublicKey(privateKey);
}

RSA::PrivateKey Crypto::LoadPrivateKeyFromBase64(const std::string& base64PrivateKey) {
    // Decode the Base64 string into raw byte data
    std::string decodedKey;
    StringSource(base64PrivateKey, true, new Base64Decoder(new StringSink(decodedKey)));

    // Load the raw data into the RSA::PrivateKey object
    RSA::PrivateKey privateKey;
    ByteQueue byteQueue;
    byteQueue.Put((const byte*)decodedKey.data(), decodedKey.size());  // Put the decoded data into byte queue
    privateKey.Load(byteQueue);  // Load the private key from the byte queue
    
    return privateKey;
}

// Function to decode a Base64 string into an RSA public key
RSA::PublicKey Crypto::LoadPublicKeyFromBase64(const std::string& base64PublicKey) {
    // Decode the Base64 string into raw byte data
    std::string decodedKey;
    StringSource(base64PublicKey, true, new Base64Decoder(new StringSink(decodedKey)));

    // Load the raw data into the RSA::PublicKey object
    RSA::PublicKey publicKey;
    ByteQueue byteQueue;
    byteQueue.Put((const byte*)decodedKey.data(), decodedKey.size());  // Put the decoded data into byte queue
    publicKey.Load(byteQueue);  // Load the public key from the byte queue
    
    return publicKey;
}

// Function to hash data (SHA-256)
void Crypto::HashData(const std::string& data, byte* hash) {
    SHA256 sha256;
    sha256.CalculateDigest(hash, (const byte*)data.data(), data.size());
}

// Function to sign the hash using RSA private key and return the signature as a Base64 string
std::string Crypto::SignData(const std::string& data, const std::string& base64PrivateKey) {
    // Load private key from Base64
    RSA::PrivateKey privateKey = LoadPrivateKeyFromBase64(base64PrivateKey);

    AutoSeededRandomPool rng;

    // Prepare the signer
    RSASSA_PKCS1v15_SHA256_Signer signer(privateKey);

    // Hash the data
    byte hash[SHA256::DIGESTSIZE];
    HashData(data, hash);  // Hashing the data (using SHA256)

    // Determine the actual size of the signature
    size_t maxSigLen = signer.MaxSignatureLength();
    byte signature[maxSigLen]; // Buffer to store the signature

    // Sign the hash
    size_t sigLen = signer.SignMessage(rng, hash, sizeof(hash), signature);

    // Convert the signature to a Base64 string
    std::string base64Signature;
    StringSource(signature, sigLen, true, new Base64Encoder(new StringSink(base64Signature), false));  // 'false' avoids line breaks

    return base64Signature;
}


// Function to verify the signature using RSA public key and Base64-encoded signature string
bool Crypto::VerifySignature(const std::string& data, const std::string& base64PublicKey, const std::string& base64Signature) {
    // Load the public key from Base64-encoded string
    RSA::PublicKey publicKey = LoadPublicKeyFromBase64(base64PublicKey);

    // Decode the Base64 signature
    std::string decodedSignature;
    StringSource(base64Signature, true, new Base64Decoder(new StringSink(decodedSignature)));

    // Prepare the verifier
    RSASSA_PKCS1v15_SHA256_Verifier verifier(publicKey);

    // Hash the data
    byte hash[SHA256::DIGESTSIZE];
    HashData(data, hash);

    // Verify the signature
    return verifier.VerifyMessage(hash, sizeof(hash), (const byte*)decodedSignature.data(), decodedSignature.size());
}

