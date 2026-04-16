#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <oqs/oqs.h> // The "Open Quantum Safe" library header

// Helper function to measure high-precision time
double get_time() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec * 1e-9;
}

int main() {
    printf("Running PQC KEM test\n");
    
    // --- INITIALIZATION ---
    // Create a new KEM object using the Kyber-768 algorithm
    // Kyber-768 is a mid-range security level (roughly equivalent to AES-192)
    OQS_KEM *kem = OQS_KEM_new(OQS_KEM_alg_kyber_768);

    if (kem == NULL) {
        printf("KEM initialization failed/n");
        return 1;
    }

    // --- MEMORY ALLOCATION ---
    // PQC algorithms use specific byte lengths for keys and secrets.
    // We allocate memory based on what the Kyber algorithm requires.
    uint8_t *public_key = malloc(kem->length_public_key);
    uint8_t *secret_key = malloc(kem->length_secret_key);
    uint8_t *ciphertext = malloc(kem->length_ciphertext);
    uint8_t *shared_secret1 = malloc(kem->length_shared_secret); // Secret created by the sender
    uint8_t *shared_secret2 = malloc(kem->length_shared_secret); // Secret recovered by the receiver

    double start;
    double end;

    // --- KEY GENERATION TEST ---
    // Generates the public key (to share) and the secret key (to keep private)
    start = get_time();
    kem->keypair(public_key, secret_key);
    end = get_time();
    printf("KeyGen time: %f seconds\n", end - start);

    // --- ENCAPSULATION TEST (The "Locking" Step) ---
    // The sender uses the receiver's public_key to create a random shared_secret1
    // and "wraps" (encapsulates) it into a ciphertext.
    
    start = get_time();
    kem->encaps(ciphertext, shared_secret1, public_key);
    end = get_time();
    printf("Encapsulation time: %f seconds\n", end - start);

    // --- DECAPSULATION TEST (The "Unlocking" Step) ---
    // The receiver uses their secret_key to "unwrap" the ciphertext 
    // and recover the same shared secret (shared_secret2).
    start = get_time();
    kem->decaps(shared_secret2, ciphertext, secret_key);
    end = get_time();
    printf("Decapsulation time: %f seconds\n", end - start);

    // --- VERIFY RESULT ---
    // Check if both secrets are exactly the same.
    // If they match, the secure "handshake" was successful.
    if (memcmp(shared_secret1, shared_secret2, kem->length_shared_secret) == 0) {
        printf("Shared secret verified\n");
    } else {
        printf("Shared secret mismatch\n");
    }

    // --- CLEANUP ---
    // Release the memory allocated for keys and secrets
    free(public_key);
    free(secret_key);
    free(ciphertext);
    free(shared_secret1);
    free(shared_secret2);

    // Release the KEM object itself
    OQS_KEM_free(kem);

    return 0;
}