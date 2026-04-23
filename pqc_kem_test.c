#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <oqs/oqs.h>

/**
 * get_time:
 * Returns monotonic time in seconds. Monotonic clock is used to avoid
 * issues with system time adjustments during the benchmark.
 */
double get_time() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec * 1e-9;
}

/**
 * print_comm_size:
 * Prints the Public Key and Ciphertext sizes. For KEMs, the 'Communication Size'
 * is the sum of these two, representing the total bytes exchanged in a handshake.
 */
void print_comm_size(const char* alg, size_t pk_size, size_t ct_size) {
    printf("%s Comm Size: PK=%zu, CT=%zu, Total=%zu bytes\n", 
            alg, pk_size, ct_size, pk_size + ct_size);
}

int main() {
    printf("Running PQC KEM Comparison Test\n");

    const char *algorithms[] = {
        OQS_KEM_alg_kyber_768,               // Lattice-based (Standard)
        OQS_KEM_alg_bike_l1,                 // Code-based
        OQS_KEM_alg_classic_mceliece_348864   // Code-based (Huge keys)
    };
    int num_algs = sizeof(algorithms) / sizeof(algorithms[0]);

    for (int i = 0; i < num_algs; i++) {
        printf("\n--- Algorithm: %s ---\n", algorithms[i]);

        // Load the algorithm metadata
        OQS_KEM *kem = OQS_KEM_new(algorithms[i]);
        if (kem == NULL) {
            printf("Initialization failed for %s\n", algorithms[i]);
            continue; 
        }

        // Allocate memory based on the specific algorithm's requirements
        uint8_t *public_key = malloc(kem->length_public_key);
        uint8_t *secret_key = malloc(kem->length_secret_key);
        uint8_t *ciphertext = malloc(kem->length_ciphertext);
        uint8_t *shared_secret1 = malloc(kem->length_shared_secret);
        uint8_t *shared_secret2 = malloc(kem->length_shared_secret);

        double start, end;

        // --- KEY GENERATION ---
        start = get_time();
        kem->keypair(public_key, secret_key);
        end = get_time();
        printf("%s KeyGen time: %f seconds\n", algorithms[i], end - start);

        // --- ENCAPSULATION ---
        start = get_time();
        kem->encaps(ciphertext, shared_secret1, public_key);
        end = get_time();
        printf("%s Encapsulation time: %f seconds\n", algorithms[i], end - start);

        // --- DECAPSULATION ---
        start = get_time();
        kem->decaps(shared_secret2, ciphertext, secret_key);
        end = get_time();
        printf("%s Decapsulation time: %f seconds\n", algorithms[i], end - start);

        // Report the 'Solid Numbers' for Communication Size
        print_comm_size(algorithms[i], kem->length_public_key, kem->length_ciphertext);

        // Clean up memory for this iteration
        free(public_key);
        free(secret_key);
        free(ciphertext);
        free(shared_secret1);
        free(shared_secret2);
        OQS_KEM_free(kem);
    }

    return 0;
}
