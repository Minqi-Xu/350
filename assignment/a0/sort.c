#include <stdio.h>
#include <time.h>

int n = 0;

void merge_sort(int l, int r, int* arr[]) {
    if(l == r)  return;
    if(l-r == 1) {
        if((*arr[l])<=(*arr[r]))
            return;
        int temp = (*arr)[l];
        (*arr)[l] = (*arr)[r];
        (*arr)[r] = temp;
        return;
    }
    int mid = (r + l) / 2;
    merge_sort(l, mid, arr);
    merge_sort(mid+1, r, arr);
    int s[n];
    int ll = l;
    int rr = mid + 1;
    int counter = l;
    while(counter <= r) {
        if(ll > mid) {
            s[counter] = (*arr)[rr];
            counter++;
            rr++;
        }
        else if(rr > r) {
            s[counter] = (*arr)[ll];
            counter++;
            ll++;
        }
        else if((*arr)[ll] < (*arr)[rr]) {
            s[counter] = (*arr)[ll];
            counter++;
            ll++;
        }
        else {
            s[counter] = (*arr)[rr];
            counter++;
            rr++;
        }
    }
    for(int i = l; i <= r; i++)
        (*arr)[i] = s[i];
}

int main() {
    FILE *f1 = fopen("log.txt", "r");
    FILE *f2 = fopen("sorted.txt", "w");

    fscanf(f1, "%d", &n);
    int arr[n];
    int* a = arr;
    for(int i = 0; i < n; i++)
        fscanf(f1, "%d", &arr[i]);


    clock_t start = clock();        // record the starting time of sorting
    merge_sort(0, n-1, &a);
    clock_t finish = clock();       // record the finishing time of sorting

    printf("Start sorting at %d\n", start);
    printf("Finish sorting at %d\n", finish);

    fprintf(f2, "%d\n", n);
    for(int i = 0; i < n; i++)
        fprintf(f2, "%d\n", arr[i]);

    fclose(f1);
    fclose(f2);

    return 0;
}
