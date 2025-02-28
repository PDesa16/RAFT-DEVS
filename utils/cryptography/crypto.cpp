#include "crypto.hpp"
// Function to convert RSA Private Key to Base64-encoded string
std::string PrivateKeyToBase64(const RSA::PrivateKey& privateKey) {
    // Convert the private key to PEM format (binary)
    std::string pemData;
    StringSink ss(pemData);
    PEM_Save(ss, privateKey);

    // Now Base64 encode the PEM format private key
    std::string base64Key;
    StringSource(pemData, true, new Base64Encoder(new StringSink(base64Key)));
    return base64Key;
}

// Function to convert RSA Public Key to Base64-encoded string
std::string PublicKeyToBase64(const RSA::PublicKey& publicKey) {
    // Convert the public key to PEM format (binary)
    std::string pemData;
    StringSink ss(pemData);
    PEM_Save(ss, publicKey);

    // Now Base64 encode the PEM format public key
    std::string base64Key;
    StringSource(pemData, true, new Base64Encoder(new StringSink(base64Key)));
    return base64Key;
}
 
// Function to create a new RSA private key
RSA::PrivateKey GeneratePrivateKey() {
    AutoSeededRandomPool rng;
    RSA::PrivateKey privateKey;
    privateKey.GenerateRandomWithKeySize(rng, 2048);
    return privateKey;
}

// Function to create RSA public key from private key
RSA::PublicKey GeneratePublicKey(const RSA::PrivateKey& privateKey) {
    return RSA::PublicKey(privateKey);
}

// Function to decode a Base64 string into an RSA private key
RSA::PrivateKey LoadPrivateKeyFromBase64(const std::string& base64PrivateKey) {
    // Decode the Base64 string into raw PEM format
    std::string decodedKey;
    StringSource(base64PrivateKey, true, new Base64Decoder(new StringSink(decodedKey)));

    // Load the PEM data into an RSA private key object
    RSA::PrivateKey privateKey;
    StringSource(decodedKey, true, new PEM_Load(new RSA::PrivateKey(privateKey)));

    return privateKey;
}

// Function to decode a Base64 string into an RSA public key
RSA::PublicKey LoadPublicKeyFromBase64(const std::string& base64PublicKey) {
    // Decode the Base64 string into raw PEM format
    std::string decodedKey;
    StringSource(base64PublicKey, true, new Base64Decoder(new StringSink(decodedKey)));

    // Load the PEM data into an RSA public key object
    RSA::PublicKey publicKey;
    StringSource(decodedKey, true, new PEM_Load(new RSA::PublicKey(publicKey)));

    return publicKey;
}

// Function to hash data (SHA-256)
void HashData(const std::string& data, byte* hash) {
    SHA256 sha256;
    sha256.CalculateDigest(hash, (const byte*)data.data(), data.size());
}

// Function to sign the hash using RSA private key and return the signature as a Base64 string
std::string SignData(const std::string& data, const std::string& base64PrivateKey) {
    // Load private key from Base64
    RSA::PrivateKey privateKey = LoadPrivateKeyFromBase64(base64PrivateKey);

    AutoSeededRandomPool rng;

    // Prepare the signer
    RSASSA_PKCS1v15_SHA256_Signer signer(privateKey);

    // Hash the data
    byte hash[SHA256::DIGESTSIZE];
    HashData(data, hash);

    // Sign the hash
    byte signature[2048]; // Buffer for the signature
    size_t sig_len = signer.MaxSignatureLength();
    signer.SignMessage(rng, hash, sizeof(hash), signature);

    // Convert the signature to a Base64 string
    std::string base64Signature;
    StringSource(signature, sig_len, true, new Base64Encoder(new StringSink(base64Signature)));
    return base64Signature;
}

// Function to verify the signature using RSA public key and Base64-encoded signature string
bool VerifySignature(const std::string& data, const std::string& base64PublicKey, const std::string& base64Signature) {
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

// int main() {
//     // Example private and public key as Base64-encoded strings (in a real system, these would come from your simulation model)
//     std::string base64PrivateKey = "MIIEpAIBAAKCAQEA7xh7K9VXtfdIY1gF2mZbD8TxS5T4sIgRV6D9HMbYPXkQTdGn0pA1FVgD1L1jW3OjjYzUkm+0CyX9L8qlx6vZChpBzF6KZ0lgf5dhqMHRX9qZ5zMH2Zk9D3aFJ1bmb4pdBdV3TAy0Ai9M6HTP5z7ZekImO2EhwGTYu6rf8uZQ+xyvJih82w8kZFVJt5cdA1YBKMNBHZHlR/R49h2cFt/nbX0Zep68XsTb6nkANP1vhxzOky7gEOa+65chQz2zJZc9hzWfaE4kcjsH+9XYPA5n0dp4IlZIeqSShFZXxuWi67P9IxttZxaOiH9OFGtr5xxGvO30+vXhZocGhszUtRHmxy5XBH1GIz8IKRxiY6q6XZXtD02jN2rzTeQAxSCyjlP7Up1ZkhxiCZyWm/s7g5mjjw1+ls4tdsCgGg5Sv1ZjF6YekyNrms5n9ytfMfFekBdX9+Z9olZmH5Gh52h8iG+id81FAv14FkZg==";
//     std::string base64PublicKey = "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA7xh7K9VXtfdIY1gF2mZbD8TxS5T4sIgRV6D9HMbYPXkQTdGn0pA1FVgD1L1jW3OjjYzUkm+0CyX9L8qlx6vZChpBzF6KZ0lgf5dhqMHRX9qZ5zMH2Zk9D3aFJ1bmb4pdBdV3TAy0Ai9M6HTP5z7ZekImO2EhwGTYu6rf8uZQ+xyvJih82w8kZFVJt5cdA1YBKMNBHZHlR/R49h2cFt/nbX0Zep68XsTb6nkANP1vhxzOky7gEOa+65chQz2zJZc9hzWfaE4kcjsH+9XYPA5n0dp4IlZIeqSShFZXxuWi67P9IxttZxaOiH9OFGtr5xxGvO30+vXhZocGhszUtRHmxy5XBH1GIz8IKRxiY6q6XZXtD02jN2rzTeQAxSCyjlP7Up1ZkhxiCZyWm/s7g5mjjw1+ls4tdsCgGg5Sv1ZjF6YekyNrms5n9ytfMfFekBdX9+Z9olZmH5Gh52h8iG+id81FAv14FkZg==";

//     // Example data to sign
//     std::string data = "Hello, world! This is a test message.";

//     // Step 1: Sign the message and get the Base64-encoded signature
//     std::string base64Signature = SignData(data, base64PrivateKey);
//     std::cout << "Base64-encoded signature: " << base64Signature << "\n";

//     // Step 2: Verify the signature using the public key and the Base64 string
//     bool isVerified = VerifySignature(data, base64PublicKey, base64Signature);
//     if (isVerified) {
//         std::cout << "Signature verified successfully!" << std::endl;
//     } else {
//         std::cout << "Signature verification failed!" << std::endl;
//     }

//     return 0;
// }
