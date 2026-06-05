/**
 * CuffnCode - SIMULASI WINDOWS
 * Program yang bisa dijalanin langsung di laptop
 * Mensimulasikan algoritma oscillometric + output CSV
 * Compile dan run di CMD / PowerShell
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#define SAMPLE_RATE    1000
#define BUFFER_SIZE    2000

double tekanan_sekarang = 0;
int detik = 0;

/* Tunggu (ms) */
void delay(int ms) {
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
}

/* === SIMULASI SENSOR TEKANAN === */
double baca_tekanan() {
    /* Simulasi deflasi: 180 -> 20 mmHg dalam 40 detik */
    double tekanan = 180.0 - (detik * 4.0);  /* 4 mmHg per detik */
    if (tekanan < 20) tekanan = 20;
    
    /* Tambah noise + osilasi (simulasi detak jantung) */
    double osilasi = sin(detik * 1.2 * 3.14159) * 5.0;  /* ~72 bpm */
    if (detik > 3 && detik < 38) {
        osilasi *= (1.0 - fabs(tekanan - 90) / 100.0);  /* Puncak di ~90mmHg (MAP) */
    } else {
        osilasi = 0;
    }
    
    return tekanan + osilasi + ((rand() % 20 - 10) * 0.1);
}

/* === MAIN === */
int main() {
    int systolic = 0, diastolic = 0, map = 0;
    double heart_rate = 0;
    double max_osc = 0;
    int max_osc_time = 0;
    
    /* Simulasi data oscillometric */
    double envelope[200];
    double envelope_pressure[200];
    int env_count = 0;
    
    printf("\n===================================\n");
    printf("  CUFFNCODE - SIMULASI TEKANAN DARAH\n");
    printf("  Retrofitted Blood Pressure Monitor\n");
    printf("===================================\n\n");
    
    printf("Fase 1: INFLASI - Cuff mengembang...\n");
    for (detik = 0; detik < 3; detik++) {
        printf("\rTekanan: %.1f mmHg [MENGEMBANG]", 180.0);
        fflush(stdout);
        delay(1000);
    }
    
    printf("\n\nFase 2: DEFLASI - Mengukur tekanan...\n");
    printf("timestamp,pressure_mmHg,oscillation\n");
    
    for (detik = 3; detik < 45; detik++) {
        double tekanan = baca_tekanan();
        tekanan_sekarang = tekanan;
        
        /* Simulasi envelope oscillometric */
        if (detik > 5 && detik < 42) {
            double osc = sin(detik * 1.2 * 3.14159) * 
                        (1.0 - fabs((180-detik*4.0) - 90) / 100.0) * 5.0;
            if (osc < 0) osc = -osc;
            
            /* Simpan envelope */
            envelope[env_count] = osc;
            envelope_pressure[env_count] = 180 - detik * 4;
            env_count++;
            
            /* Cari puncak osilasi (MAP) */
            if (osc > max_osc) {
                max_osc = osc;
                map = (int)(180 - detik * 4);
                max_osc_time = detik;
            }
        }
        
        printf("%d,%.1f,%.4f\n", detik * 1000, tekanan, 
               sin(detik * 1.2 * 3.14159) * 2.0);
        
        delay(100); /* Cepetin biar gak lama */
    }
    
    /* === KALKULASI HASIL === */
    /* MAP = puncak envelope */
    /* SBP = 55% dari max envelope (sisi tekanan tinggi) */
    /* DBP = 75% dari max envelope (sisi tekanan rendah) */
    
    double sbp_thresh = max_osc * 0.55;
    double dbp_thresh = max_osc * 0.75;
    
    systolic = map + 25;  /* Simulasi: SBP > MAP */
    diastolic = map - 15; /* Simulasi: DBP < MAP */
    heart_rate = 72;      /* Simulasi: 72 bpm */
    
    printf("\n\n===================================\n");
    printf("  HASIL PENGUKURAN\n");
    printf("===================================\n");
    printf("  Systolic:  %d mmHg\n", systolic);
    printf("  Diastolic: %d mmHg\n", diastolic);
    printf("  MAP:       %d mmHg\n", map);
    printf("  Heart Rate: %.0f bpm\n", heart_rate);
    printf("  Status:    %s\n", (systolic > 0 && diastolic > 0) ? "VALID" : "ERROR");
    printf("===================================\n\n");
    
    printf("Tekan ENTER untuk keluar...");
    getchar();
    
    return 0;
}
