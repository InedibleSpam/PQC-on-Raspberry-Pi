#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <oqs/oqs.h>

// Standard helper to measure high-precision time in seconds
double get_time() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec * 1e-9;
}

int main() {
    printf("Running PQC KEM Comparison Test\n");

    // We store the algorithm names in an array so we can loop through them
    const char *algorithms[] = {
        OQS_KEM_alg_kyber_768,             // NIST winner (Lattice-based)
        OQS_KEM_alg_bike_l1,               // Code-based (Error-correcting codes)
        OQS_KEM_alg_classic_mceliece_348864 // The "old reliable" of PQC
    };
    int num_algs = sizeof(algorithms) / sizeof(algorithms[0]);

    // Loop through each algorithm one by one
    for (int i = 0; i < num_algs; i++) {
        printf("\n--- Algorithm: %s ---\n", algorithms[i]);

        // --- INITIALIZATION ---
        // Load the specific math instructions for the current algorithm
        OQS_KEM *kem = OQS_KEM_new(algorithms[i]);
        if (kem == NULL) {
            printf("Initialization failed for %s\n", algorithms[i]);
            continue; 
        }

        // --- MEMORY ALLOCATION ---
        // Every PQC algorithm has different key sizes. 
        // We use the 'kem' object to tell us exactly how much memory to 'malloc'.
        uint8_t *public_key = malloc(kem->length_public_key);
        uint8_t *secret_key = malloc(kem->length_secret_key);
        uint8_t *ciphertext = malloc(kem->length_ciphertext);
        uint8_t *shared_secret1 = malloc(kem->length_shared_secret); // Secret for SENDER
        uint8_t *shared_secret2 = malloc(kem->length_shared_secret); // Secret for RECEIVER

        double start, end;

        // --- KEY GENERATION ---
        // Create the public key (to send out) and secret key (to keep hidden)
        start = get_time();
        kem->keypair(public_key, secret_key);
        end = get_time();
        printf("%s KeyGen time: %f seconds\n", algorithms[i], end - start);

        // --- ENCAPSULATION ---
        // SENDER: Uses the public key to create a secret and hide it in a "ciphertext" box
        start = get_time();
        kem->encaps(ciphertext, shared_secret1, public_key);
        end = get_time();
        printf("%s Encapsulation time: %f seconds\n", algorithms[i], end - start);

        // --- DECAPSULATION ---
        // RECEIVER: Uses their secret key to open the "ciphertext" box and get the secret
        start = get_time();
        kem->decaps(shared_secret2, ciphertext, secret_key);
        end = get_time();
        printf("%s Decapsulation time: %f seconds\n", algorithms[i], end - start);

        // --- VERIFY ---
        // Make sure both parties ended up with the exact same secret key
        if (memcmp(shared_secret1, shared_secret2, kem->length_shared_secret) == 0) {
            printf("Result: Shared secret verified!\n");
        } else {
            printf("Result: ERROR - Shared secret mismatch!\n");
        }

        // --- CLEANUP ---
        // Free the memory for this algorithm before moving to the next one
        free(public_key);
        free(secret_key);
        free(ciphertext);
        free(shared_secret1);
        free(shared_secret2);
        OQS_KEM_free(kem);
    }

    return 0;
}
