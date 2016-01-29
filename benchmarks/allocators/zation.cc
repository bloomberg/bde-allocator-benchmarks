#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <stdint.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <iostream>
#include <bdlma_sequentialallocator.h>
#include <bdlma_multipoolallocator.h>
#include <bslma_newdeleteallocator.h>
#include <bsl_vector.h>

using namespace BloombergLP;

#define ITER 7

int64_t totalAllocation;
int64_t activeAllocation;
int64_t allocationSize;


struct directAllocator {
    void *allocate(size_t s) {  return ::operator new(s);  }
    void deallocate(void *p) {  ::operator delete(p);  }
};

template <class T>
double testOnce(T& allocator) {
    timespec start;
    timespec stop;

    bsl::vector<void *> active;
    active.resize(activeAllocation);

    clock_gettime(CLOCK_MONOTONIC, &start);

    int64_t i;
    for (i = 0; i < totalAllocation && i < activeAllocation; ++i) {
        active[i] = allocator.allocate(allocationSize);
        ++(*static_cast<char *>(active[i]));
    }
    for (; i < totalAllocation; ++i) {
        int64_t j = i % activeAllocation;
        allocator.deallocate(active[j]);
        active[j] = allocator.allocate(allocationSize);
        ++(*static_cast<char *>(active[j]));
    }
    for (i = 0; i < totalAllocation && i < activeAllocation; ++i) {
        allocator.deallocate(active[i]);
    }

    clock_gettime(CLOCK_MONOTONIC, &stop);
    int64_t rv = static_cast<int64_t>(stop.tv_sec - start.tv_sec) * 1000000000LL
                    + (stop.tv_nsec - start.tv_nsec);
    return (static_cast<double>(rv) / 1000000000.0L);
}

template <>
double testOnce(bdlma::SequentialPool& allocator) {
    timespec start;
    timespec stop;

    bsl::vector<void *> active;
    active.resize(activeAllocation);

    clock_gettime(CLOCK_MONOTONIC, &start);

    int64_t i;
    for (i = 0; i < totalAllocation && i < activeAllocation; ++i) {
        active[i] = allocator.allocate(allocationSize);
        ++(*static_cast<char *>(active[i]));
    }
    for (; i < totalAllocation; ++i) {
        int64_t j = i % activeAllocation;
        active[j] = allocator.allocate(allocationSize);
        ++(*static_cast<char *>(active[j]));
    }
    for (i = 0; i < totalAllocation && i < activeAllocation; ++i) {
    }

    clock_gettime(CLOCK_MONOTONIC, &stop);
    int64_t rv = static_cast<int64_t>(stop.tv_sec - start.tv_sec) * 1000000000LL
                    + (stop.tv_nsec - start.tv_nsec);
    return (static_cast<double>(rv) / 1000000000.0L);
}

template <class T1, class T2>
double test() {
    if (ITER == 1) {
        T2 alloc;
        T1 allocator(&alloc);
        return testOnce(allocator);
    }

    std::vector<double> v;
    for (int i = 0; i < ITER; ++i) {
        T2 alloc;
        T1 allocator(&alloc);
        v.push_back(testOnce(allocator));
    }

    if (ITER < 7) {
        double t = 0;
        for (unsigned i = 0; i < v.size(); ++i) {
            t += v[i];
        }
        return t / v.size();
    }


    std::sort(v.begin(), v.end());

    double t = 0;
    v.pop_back();
    for (unsigned i = 1; i < v.size(); ++i) {
        t += v[i];
    }
    return t / (v.size() - 1);
}

template <class T1>
double test() {
    if (ITER == 1) {
        T1 allocator;
        return testOnce(allocator);
    }

    std::vector<double> v;
    for (int i = 0; i < ITER; ++i) {
        T1 allocator;
        v.push_back(testOnce(allocator));
    }

    if (ITER < 7) {
        double t = 0;
        for (unsigned i = 0; i < v.size(); ++i) {
            t += v[i];
        }
        return t / v.size();
    }

    std::sort(v.begin(), v.end());

    double t = 0;
    v.pop_back();
    for (unsigned i = 1; i < v.size(); ++i) {
        t += v[i];
    }
    return t / (v.size() - 1);
}

template <class T1, class T2>
double childTest() {
    int fd[2];
    if (-1 == pipe(fd)) {
        printf("could not open pipe\n");
        exit(0);
    }
    pid_t pid = fork();
    if (0 == pid) {
        double rv = test<T1, T2>();
        write(fd[1], &rv, sizeof rv);
        close(fd[0]);
        close(fd[1]);
        exit(0);
    }
    int status;
    wait(&status);
    double rv = -1.0;
    if (WIFEXITED(status)) {
        read(fd[0], &rv, sizeof rv);
    }
    close(fd[0]);
    close(fd[1]);
    return rv;
}

template <class T1>
double childTest() {
    int fd[2];
    if (-1 == pipe(fd)) {
        printf("could not open pipe\n");
        exit(0);
    }
    pid_t pid = fork();
    if (0 == pid) {
        double rv = test<T1>();
        write(fd[1], &rv, sizeof rv);
        close(fd[0]);
        close(fd[1]);
        exit(0);
    }
    int status;
    wait(&status);
    double rv = -1.0;
    if (WIFEXITED(status)) {
        read(fd[0], &rv, sizeof rv);
    }
    close(fd[0]);
    close(fd[1]);
    return rv;
}

int main(int argc, char *argv[]) {
    int totalSize = argc > 1 ? atoi(argv[1]) : 10;
    int activeSize = argc > 2 ? atoi(argv[2]) : 6;
    int blockSize = argc > 3 ? atoi(argv[3]) : 4;

    totalAllocation = 1LL << (totalSize - blockSize);
    activeAllocation =  1LL << (activeSize - blockSize);
    allocationSize =  1LL << blockSize;

    printf("%i,%i,%i", totalSize, activeSize, blockSize);
    fflush(stdout);

    int status;
    double firstRV;
    for (int allocatorIndex = 0; allocatorIndex < 8; ++allocatorIndex) {
        double rv = -1.0;
        switch (allocatorIndex) {
          case 0: {
            rv = childTest<directAllocator>();
            firstRV = rv;
            printf(",%0.3lfs", rv);
            fflush(stdout);
          } break;
          case 1: {
            rv = childTest<bslma::NewDeleteAllocator>();
            printf(",%0.0lf", 100.0 * rv / firstRV);
            fflush(stdout);
          } break;
          case 2: {
            rv = childTest<bdlma::SequentialPool>();
            if (rv != -1.0) printf(",%0.0lf", 100.0 * rv / firstRV);
            else printf(",fail");
            fflush(stdout);
          } break;
          case 3: {
            rv = childTest<bdlma::SequentialAllocator>();
            if (rv != -1.0) printf(",%0.0lf", 100.0 * rv / firstRV);
            else printf(",fail");
            fflush(stdout);
          } break;
          case 4: {
            rv = childTest<bdlma::Multipool>();
            if (rv != -1.0) printf(",%0.0lf", 100.0 * rv / firstRV);
            else printf(",fail");
            fflush(stdout);
          } break;
          case 5: {
            rv = childTest<bdlma::MultipoolAllocator>();
            if (rv != -1.0) printf(",%0.0lf", 100.0 * rv / firstRV);
            else printf(",fail");
            fflush(stdout);
          } break;
          case 6: {
            rv = childTest<bdlma::Multipool, bdlma::SequentialAllocator>();
            if (rv != -1.0) printf(",%0.0lf", 100.0 * rv / firstRV);
            else printf(",fail");
            fflush(stdout);
          } break;
          case 7: {
            rv = childTest<bdlma::MultipoolAllocator,
                           bdlma::SequentialAllocator>();
            if (rv != -1.0) printf(",%0.0lf", 100.0 * rv / firstRV);
            else printf(",fail");
            fflush(stdout);
          } break;
        }
    }
    printf("\n");
    return 0;
}
