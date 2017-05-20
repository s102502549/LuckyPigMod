/*
 * Created by g8456 on 2017/5/7.
 * Usage read pid mode type operation value
 *
 * mode
 * 'n' for first search, in this mode, type must be 'e' or 'n'
 * 's' for following search, in this mode, type must be 'e', 'l', 'b', 'u', 'c'
 *
 * type
 * 'b' for 1 byte integer
 * 's' for 2 bytes integer
 * 'i' for 4 bytes integer
 * 'li'for 8 bytes integer
 * 'f' for float
 * 'd' for double
 *
 * operation
 * 'e' equal
 * 'l' less than
 * 'b' bigger than
 * 'n' no condition
 * 'u' unchange
 * 'c' change
*/

#include <sys/ptrace.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <list>

#define argPid argv[1]
#define argMode argv[2]
#define argType argv[3]
#define argOperation argv[4]
#define argValue argv[5]

union Data {
    unsigned char bytes[8];
    unsigned char c;
    unsigned short s;
    unsigned int i;
    unsigned long long int li;
    float f;
    double d;
};

struct Region {
    off64_t start;
    off64_t end;
};

int isNeedToScan(char *permission, char *path) {
    return strstr(permission, "w") &&
           (path == NULL || strstr(path, "anon:linker_alloc") || strstr(path, "anon:libc_malloc")||strstr(path, "[heap]"));
}

void generateList(FILE *result, FILE *list) {
    char buffer[4096];
    int numRead;
    fseek(result, 0, SEEK_SET);
    while ((numRead = fread(buffer, 1, sizeof(buffer), result)) > 0) {
        fwrite(buffer, 1, numRead, list);
    }
}

void generateDisplay(FILE *result, FILE *display, char *type) {
    Data data;
    unsigned int totalSize;
    int count = 0;
    fseek(result, -4, SEEK_END);
    fread(&totalSize, sizeof(totalSize), 1, result);
    fseek(result, 0, SEEK_SET);
    if (strcmp(type, "b") == 0) {
        for (unsigned int i = 0; count < 100 && i < totalSize; i++, count++) {
            fread(data.bytes, 8, 1, result);
            fprintf(display, "%08llx ", data.li);
            fread(data.bytes, 8, 1, result);
            fprintf(display, "%hhu\n", data.c);
        }
    } else if (strcmp(type, "s") == 0) {
        for (unsigned int i = 0; count < 100 && i < totalSize; i++, count++) {
            fread(data.bytes, 8, 1, result);
            fprintf(display, "%08llx ", data.li);
            fread(data.bytes, 8, 1, result);
            fprintf(display, "%hu\n", data.s);
        }
    } else if (strcmp(type, "i") == 0) {
        for (unsigned int i = 0; count < 100 && i < totalSize; i++, count++) {
            fread(data.bytes, 8, 1, result);
            fprintf(display, "%08llx ", data.li);
            fread(data.bytes, 8, 1, result);
            fprintf(display, "%u\n", data.i);
        }
    } else if (strcmp(type, "li") == 0) {
        for (unsigned int i = 0; count < 100 && i < totalSize; i++, count++) {
            fread(data.bytes, 8, 1, result);
            fprintf(display, "%08llx ", data.li);
            fread(data.bytes, 8, 1, result);
            fprintf(display, "%llu\n", data.li);
        }
    } else if (strcmp(type, "f") == 0) {
        for (unsigned int i = 0; count < 100 && i < totalSize; i++, count++) {
            fread(data.bytes, 8, 1, result);
            fprintf(display, "%08llx ", data.li);
            fread(data.bytes, 8, 1, result);
            fprintf(display, "%f\n", data.f);
        }
    } else if (strcmp(type, "d") == 0) {
        for (unsigned int i = 0; count < 100 && i < totalSize; i++, count++) {
            fread(data.bytes, 8, 1, result);
            fprintf(display, "%08llx ", data.li);
            fread(data.bytes, 8, 1, result);
            fprintf(display, "%f\n", data.d);
        }
    }
}

int main(int argc, char *argv[]) {
    pid_t pid = strtol(argPid, NULL, NULL);
    char memPath[20];
    char mapsPath[20];
    char listFilePath[60];
    char resultFilePath[60];
    char displayFilePath[60];
    char buffer[1024];
    Data data, tmpData;
    unsigned int totalSize = 0, currentSize = 0, count = 0;
    std::list<Region> regionList;
    int memFd;
    FILE *maps, *list, *result, *display;
    char packageName[] = "com.pigfood.luckypig.luckypigmod";

    //construct file path
    snprintf(memPath, sizeof(memPath), "/proc/%d/mem", pid);
    snprintf(mapsPath, sizeof(mapsPath), "/proc/%d/maps", pid);
    snprintf(listFilePath, sizeof(listFilePath), "/data/data/%s/files/list", packageName);
    snprintf(resultFilePath, sizeof(resultFilePath), "/data/data/%s/files/result", packageName);
    snprintf(displayFilePath, sizeof(displayFilePath), "/data/data/%s/files/display", packageName);

    if (argValue == NULL) {
        printf("null\n");
        argValue = "2";
    }

    //store value
    if (strcmp(argType, "b") == 0) {
        data.c = strtoull(argValue, NULL, NULL);
    } else if (strcmp(argType, "s") == 0) {
        data.s = strtoull(argValue, NULL, NULL);
    } else if (strcmp(argType, "i") == 0) {
        data.i = strtoull(argValue, NULL, NULL);
    } else if (strcmp(argType, "li") == 0) {
        data.li = strtoull(argValue, NULL, NULL);
    } else if (strcmp(argType, "f") == 0) {
        data.f = strtof(argValue, NULL);
    } else if (strcmp(argType, "d") == 0) {
        data.d = strtod(argValue, NULL);
    }


    //use ptrace to attach to process
    ptrace(PTRACE_ATTACH, pid, NULL, NULL);
    waitpid(pid, NULL, NULL);

    //open file for search
    memFd = open(memPath, O_RDONLY);
    //result = fopen(resultFilePath, "wb+");
    result = tmpfile();
    if (result == NULL) {
        printf("failed\n");
        return -1;
    }
    //search value, and print to file
    //first search, open maps to see what regions to search
    if (strcmp(argMode, "n") == 0) {
        maps = fopen(mapsPath, "r");
        while (fgets(buffer, sizeof(buffer), maps)) {
            char *region, *permission, *offset, *device, *inode, *path;
            //split each item
            region = strtok(buffer, " \n");
            permission = strtok(NULL, " \n");
            offset = strtok(NULL, " \n");
            device = strtok(NULL, " \n");
            inode = strtok(NULL, " \n");
            path = strtok(NULL, " \n");
            //split region
            Region address;
            address.start = strtoll(strtok(region, "-"), NULL, 16);
            address.end = strtoll(strtok(NULL, "-"), NULL, 16);
            //check if need to scan, if true, calculate region size and add region to region list
            if (isNeedToScan(permission, path)) {
                //printf("%s %s %s %s %s %s %llx %llx\n", region, permission, offset, device, inode,
                //path, address.start, address.end);
                regionList.push_back(address);
                totalSize += address.end - address.start;
            }
        }
        printf("%u\n", totalSize);
        //scan and print progress
        //first, exact
        if (strcmp(argOperation, "e") == 0) {
            if (strcmp(argType, "b") == 0) {
                for (std::list<Region>::iterator it = regionList.begin();
                     it != regionList.end(); it++) {
                    Region tmpRegion = *it;
                    for (; tmpRegion.start < tmpRegion.end; tmpRegion.start += 1) {
                        pread64(memFd, tmpData.bytes, 1, tmpRegion.start);
                        if (tmpData.c == data.c) {
                            fwrite(&tmpRegion.start, 8, 1, result);
                            fwrite(&tmpData.bytes, 8, 1, result);
                            currentSize++;
                        }
                        count += 1;
                    }
                    printf("%u\n", count);
                }
            } else if (strcmp(argType, "s") == 0) {
                for (std::list<Region>::iterator it = regionList.begin();
                     it != regionList.end(); it++) {
                    Region tmpRegion = *it;
                    for (; tmpRegion.start < tmpRegion.end; tmpRegion.start += 2) {
                        pread64(memFd, tmpData.bytes, 2, tmpRegion.start);
                        if (tmpData.s == data.s) {
                            fwrite(&tmpRegion.start, 8, 1, result);
                            fwrite(&tmpData.bytes, 8, 1, result);
                            currentSize++;
                        }
                        count += 2;
                    }
                    printf("%u\n", count);
                }
            } else if (strcmp(argType, "i") == 0) {
                for (std::list<Region>::iterator it = regionList.begin();
                     it != regionList.end(); it++) {
                    Region tmpRegion = *it;
                    for (; tmpRegion.start < tmpRegion.end; tmpRegion.start += 4) {
                        pread64(memFd, tmpData.bytes, 4, tmpRegion.start);
                        if (tmpData.i == data.i) {
                            fwrite(&tmpRegion.start, 8, 1, result);
                            fwrite(&tmpData.bytes, 8, 1, result);
                            currentSize++;
                        }
                        count += 4;
                    }
                    printf("%u\n", count);
                }
            } else if (strcmp(argType, "li") == 0) {
                for (std::list<Region>::iterator it = regionList.begin();
                     it != regionList.end(); it++) {
                    Region tmpRegion = *it;
                    for (; tmpRegion.start < tmpRegion.end; tmpRegion.start += 8) {
                        pread64(memFd, tmpData.bytes, 8, tmpRegion.start);
                        if (tmpData.li == data.li) {
                            fwrite(&tmpRegion.start, 8, 1, result);
                            fwrite(&tmpData.bytes, 8, 1, result);
                            currentSize++;
                        }
                        count += 8;
                    }
                    printf("%u\n", count);
                }
            } else if (strcmp(argType, "f") == 0) {
                for (std::list<Region>::iterator it = regionList.begin();
                     it != regionList.end(); it++) {
                    Region tmpRegion = *it;
                    for (; tmpRegion.start < tmpRegion.end; tmpRegion.start += 4) {
                        pread64(memFd, tmpData.bytes, 4, tmpRegion.start);
                        if (tmpData.f == data.f) {
                            fwrite(&tmpRegion.start, 8, 1, result);
                            fwrite(&tmpData.bytes, 8, 1, result);
                            currentSize++;
                        }
                        count += 4;
                    }
                    printf("%u\n", count);
                }
            } else if (strcmp(argType, "d") == 0) {
                for (std::list<Region>::iterator it = regionList.begin();
                     it != regionList.end(); it++) {
                    Region tmpRegion = *it;
                    for (; tmpRegion.start < tmpRegion.end; tmpRegion.start += 8) {
                        pread64(memFd, tmpData.bytes, 8, tmpRegion.start);
                        if (tmpData.d == data.d) {
                            fwrite(&tmpRegion.start, 8, 1, result);
                            fwrite(&tmpData.bytes, 8, 1, result);
                            currentSize++;
                        }
                        count += 8;
                    }
                    printf("%u\n", count);
                }
            }
        }//first, no condition
        else if (strcmp(argOperation, "n") == 0) {
            if (strcmp(argType, "b") == 0) {
                for (std::list<Region>::iterator it = regionList.begin();
                     it != regionList.end(); it++) {
                    Region tmpRegion = *it;
                    for (; tmpRegion.start < tmpRegion.end; tmpRegion.start += 1) {
                        pread64(memFd, tmpData.bytes, 1, tmpRegion.start);
                        fwrite(&tmpRegion.start, 8, 1, result);
                        fwrite(&tmpData.bytes, 8, 1, result);
                        currentSize++;
                        count += 1;
                    }
                    printf("%u\n", count);
                }
            } else if (strcmp(argType, "s") == 0) {
                for (std::list<Region>::iterator it = regionList.begin();
                     it != regionList.end(); it++) {
                    Region tmpRegion = *it;
                    for (; tmpRegion.start < tmpRegion.end; tmpRegion.start += 2) {
                        pread64(memFd, tmpData.bytes, 2, tmpRegion.start);
                        fwrite(&tmpRegion.start, 8, 1, result);
                        fwrite(&tmpData.bytes, 8, 1, result);
                        currentSize++;
                        count += 2;
                    }
                    printf("%u\n", count);
                }
            } else if (strcmp(argType, "i") == 0) {
                for (std::list<Region>::iterator it = regionList.begin();
                     it != regionList.end(); it++) {
                    Region tmpRegion = *it;
                    for (; tmpRegion.start < tmpRegion.end; tmpRegion.start += 4) {
                        pread64(memFd, tmpData.bytes, 4, tmpRegion.start);
                        fwrite(&tmpRegion.start, 8, 1, result);
                        fwrite(&tmpData.bytes, 8, 1, result);
                        currentSize++;
                        count += 4;
                    }
                    printf("%u\n", count);
                }
            } else if (strcmp(argType, "li") == 0) {
                for (std::list<Region>::iterator it = regionList.begin();
                     it != regionList.end(); it++) {
                    Region tmpRegion = *it;
                    for (; tmpRegion.start < tmpRegion.end; tmpRegion.start += 8) {
                        pread64(memFd, tmpData.bytes, 8, tmpRegion.start);
                        fwrite(&tmpRegion.start, 8, 1, result);
                        fwrite(&tmpData.bytes, 8, 1, result);
                        currentSize++;
                        count += 8;
                    }
                    printf("%u\n", count);
                }
            } else if (strcmp(argType, "f") == 0) {
                for (std::list<Region>::iterator it = regionList.begin();
                     it != regionList.end(); it++) {
                    Region tmpRegion = *it;
                    for (; tmpRegion.start < tmpRegion.end; tmpRegion.start += 4) {
                        pread64(memFd, tmpData.bytes, 4, tmpRegion.start);
                        fwrite(&tmpRegion.start, 8, 1, result);
                        fwrite(&tmpData.bytes, 8, 1, result);
                        currentSize++;
                        count += 4;
                    }
                    printf("%u\n", count);
                }
            } else if (strcmp(argType, "d") == 0) {
                for (std::list<Region>::iterator it = regionList.begin();
                     it != regionList.end(); it++) {
                    Region tmpRegion = *it;
                    for (; tmpRegion.start < tmpRegion.end; tmpRegion.start += 8) {
                        pread64(memFd, tmpData.bytes, 8, tmpRegion.start);
                        fwrite(&tmpRegion.start, 8, 1, result);
                        fwrite(&tmpData.bytes, 8, 1, result);
                        currentSize++;
                        count += 8;
                    }
                    printf("%u\n", count);
                }
            }
        }
        //generate result
        fwrite(&currentSize, sizeof(currentSize), 1, result);//totalSize
    }//following search, open list to see what addresses to search
    else if (strcmp(argMode, "s") == 0) {
        Data address;
        list = fopen(listFilePath, "r");
        //get total element num in result
        fseek(list, -4, SEEK_END);
        fread(&totalSize, sizeof(totalSize), 1, list);
        fseek(list, 0, SEEK_SET);
        printf("%u\n", totalSize);
        //scan
        if (strcmp(argOperation, "e") == 0) {
            if (strcmp(argType, "b") == 0) {
                for (unsigned int i = 0; i < totalSize; i++) {
                    fread(address.bytes, 8, 1, list);//address
                    pread64(memFd, tmpData.bytes, 1, address.li);
                    if (tmpData.c == data.c) {
                        fwrite(address.bytes, 8, 1, result);
                        fwrite(tmpData.bytes, 8, 1, result);
                        currentSize++;
                    }
                    fread(tmpData.bytes, 8, 1, list);//discard value
                    count++;
//                    if (count % 100 == 0)
//                        printf("%u\n", count);
                }
            } else if (strcmp(argType, "s") == 0) {
                for (unsigned int i = 0; i < totalSize; i++) {
                    fread(address.bytes, 8, 1, list);//address
                    pread64(memFd, tmpData.bytes, 2, address.li);
                    if (tmpData.s == data.s) {
                        fwrite(address.bytes, 8, 1, result);
                        fwrite(tmpData.bytes, 8, 1, result);
                        currentSize++;
                    }
                    fread(tmpData.bytes, 8, 1, list);//discard value
                    count++;
//                    if (count % 100 == 0)
//                        printf("%u\n", count);
                }
            } else if (strcmp(argType, "i") == 0) {
                for (unsigned int i = 0; i < totalSize; i++) {
                    fread(address.bytes, 8, 1, list);//address
                    pread64(memFd, tmpData.bytes, 4, address.li);
                    if (tmpData.i == data.i) {
                        fwrite(address.bytes, 8, 1, result);
                        fwrite(tmpData.bytes, 8, 1, result);
                        currentSize++;
                    }
                    fread(tmpData.bytes, 8, 1, list);//discard value
                    count++;
//                    if (count % 100 == 0)
//                        printf("%u\n", count);
                }
            } else if (strcmp(argType, "li") == 0) {
                for (unsigned int i = 0; i < totalSize; i++) {
                    fread(address.bytes, 8, 1, list);//address
                    pread64(memFd, tmpData.bytes, 8, address.li);
                    if (tmpData.li == data.li) {
                        fwrite(address.bytes, 8, 1, result);
                        fwrite(tmpData.bytes, 8, 1, result);
                        currentSize++;
                    }
                    fread(tmpData.bytes, 8, 1, list);//discard value
                    count++;
//                    if (count % 100 == 0)
//                        printf("%u\n", count);
                }
            } else if (strcmp(argType, "f") == 0) {
                for (unsigned int i = 0; i < totalSize; i++) {
                    fread(address.bytes, 8, 1, list);//address
                    pread64(memFd, tmpData.bytes, 4, address.li);
                    if (tmpData.f == data.f) {
                        fwrite(address.bytes, 8, 1, result);
                        fwrite(tmpData.bytes, 8, 1, result);
                        currentSize++;
                    }
                    fread(tmpData.bytes, 8, 1, list);//discard value
                    count++;
//                    if (count % 100 == 0)
//                        printf("%u\n", count);
                }
            } else if (strcmp(argType, "d") == 0) {
                for (unsigned int i = 0; i < totalSize; i++) {
                    fread(address.bytes, 8, 1, list);//address
                    pread64(memFd, tmpData.bytes, 8, address.li);
                    if (tmpData.d == data.d) {
                        fwrite(address.bytes, 8, 1, result);
                        fwrite(tmpData.bytes, 8, 1, result);
                        currentSize++;
                    }
                    fread(tmpData.bytes, 8, 1, list);//discard value
                    count++;
//                    if (count % 100 == 0)
//                        printf("%u\n", count);
                }
            }
        } else if (strcmp(argOperation, "l") == 0) {
            if (strcmp(argType, "b") == 0) {
                for (unsigned int i = 0; i < totalSize; i++) {
                    fread(address.bytes, 8, 1, list);//address
                    pread64(memFd, tmpData.bytes, 1, address.li);
                    if (tmpData.c < data.c) {
                        fwrite(address.bytes, 8, 1, result);
                        fwrite(tmpData.bytes, 8, 1, result);
                        currentSize++;
                    }
                    fread(tmpData.bytes, 8, 1, list);//discard value
                    count++;
//                    if (count % 100 == 0)
//                        printf("%u\n", count);
                }
            } else if (strcmp(argType, "s") == 0) {
                for (unsigned int i = 0; i < totalSize; i++) {
                    fread(address.bytes, 8, 1, list);//address
                    pread64(memFd, tmpData.bytes, 2, address.li);
                    if (tmpData.s < data.s) {
                        fwrite(address.bytes, 8, 1, result);
                        fwrite(tmpData.bytes, 8, 1, result);
                        currentSize++;
                    }
                    fread(tmpData.bytes, 8, 1, list);//discard value
                    count++;
//                    if (count % 100 == 0)
//                        printf("%u\n", count);
                }
            } else if (strcmp(argType, "i") == 0) {
                for (unsigned int i = 0; i < totalSize; i++) {
                    fread(address.bytes, 8, 1, list);//address
                    pread64(memFd, tmpData.bytes, 4, address.li);
                    if (tmpData.i < data.i) {
                        fwrite(address.bytes, 8, 1, result);
                        fwrite(tmpData.bytes, 8, 1, result);
                        currentSize++;
                    }
                    fread(tmpData.bytes, 8, 1, list);//discard value
                    count++;
//                    if (count % 100 == 0)
//                        printf("%u\n", count);
                }
            } else if (strcmp(argType, "li") == 0) {
                for (unsigned int i = 0; i < totalSize; i++) {
                    fread(address.bytes, 8, 1, list);//address
                    pread64(memFd, tmpData.bytes, 8, address.li);
                    if (tmpData.li < data.li) {
                        fwrite(address.bytes, 8, 1, result);
                        fwrite(tmpData.bytes, 8, 1, result);
                        currentSize++;
                    }
                    fread(tmpData.bytes, 8, 1, list);//discard value
                    count++;
//                    if (count % 100 == 0)
//                        printf("%u\n", count);
                }
            } else if (strcmp(argType, "f") == 0) {
                for (unsigned int i = 0; i < totalSize; i++) {
                    fread(address.bytes, 8, 1, list);//address
                    pread64(memFd, tmpData.bytes, 4, address.li);
                    if (tmpData.f < data.f) {
                        fwrite(address.bytes, 8, 1, result);
                        fwrite(tmpData.bytes, 8, 1, result);
                        currentSize++;
                    }
                    fread(tmpData.bytes, 8, 1, list);//discard value
                    count++;
//                    if (count % 100 == 0)
//                        printf("%u\n", count);
                }
            } else if (strcmp(argType, "d") == 0) {
                for (unsigned int i = 0; i < totalSize; i++) {
                    fread(address.bytes, 8, 1, list);//address
                    pread64(memFd, tmpData.bytes, 8, address.li);
                    if (tmpData.d < data.d) {
                        fwrite(address.bytes, 8, 1, result);
                        fwrite(tmpData.bytes, 8, 1, result);
                        currentSize++;
                    }
                    fread(tmpData.bytes, 8, 1, list);//discard value
                    count++;
//                    if (count % 100 == 0)
//                        printf("%u\n", count);
                }
            }
        } else if (strcmp(argOperation, "b") == 0) {
            if (strcmp(argType, "b") == 0) {
                for (unsigned int i = 0; i < totalSize; i++) {
                    fread(address.bytes, 8, 1, list);//address
                    pread64(memFd, tmpData.bytes, 1, address.li);
                    if (tmpData.c > data.c) {
                        fwrite(address.bytes, 8, 1, result);
                        fwrite(tmpData.bytes, 8, 1, result);
                        currentSize++;
                    }
                    fread(tmpData.bytes, 8, 1, list);//discard value
                    count++;
//                    if (count % 100 == 0)
//                        printf("%u\n", count);
                }
            } else if (strcmp(argType, "s") == 0) {
                for (unsigned int i = 0; i < totalSize; i++) {
                    fread(address.bytes, 8, 1, list);//address
                    pread64(memFd, tmpData.bytes, 2, address.li);
                    if (tmpData.s > data.s) {
                        fwrite(address.bytes, 8, 1, result);
                        fwrite(tmpData.bytes, 8, 1, result);
                        currentSize++;
                    }
                    fread(tmpData.bytes, 8, 1, list);//discard value
                    count++;
//                    if (count % 100 == 0)
//                        printf("%u\n", count);
                }
            } else if (strcmp(argType, "i") == 0) {
                for (unsigned int i = 0; i < totalSize; i++) {
                    fread(address.bytes, 8, 1, list);//address
                    pread64(memFd, tmpData.bytes, 4, address.li);
                    if (tmpData.i > data.i) {
                        fwrite(address.bytes, 8, 1, result);
                        fwrite(tmpData.bytes, 8, 1, result);
                        currentSize++;
                    }
                    fread(tmpData.bytes, 8, 1, list);//discard value
                    count++;
//                    if (count % 100 == 0)
//                        printf("%u\n", count);
                }
            } else if (strcmp(argType, "li") == 0) {
                for (unsigned int i = 0; i < totalSize; i++) {
                    fread(address.bytes, 8, 1, list);//address
                    pread64(memFd, tmpData.bytes, 8, address.li);
                    if (tmpData.li > data.li) {
                        fwrite(address.bytes, 8, 1, result);
                        fwrite(tmpData.bytes, 8, 1, result);
                        currentSize++;
                    }
                    fread(tmpData.bytes, 8, 1, list);//discard value
                    count++;
//                    if (count % 100 == 0)
//                        printf("%u\n", count);
                }
            } else if (strcmp(argType, "f") == 0) {
                for (unsigned int i = 0; i < totalSize; i++) {
                    fread(address.bytes, 8, 1, list);//address
                    pread64(memFd, tmpData.bytes, 4, address.li);
                    if (tmpData.f > data.f) {
                        fwrite(address.bytes, 8, 1, result);
                        fwrite(tmpData.bytes, 8, 1, result);
                        currentSize++;
                    }
                    fread(tmpData.bytes, 8, 1, list);//discard value
                    count++;
//                    if (count % 100 == 0)
//                        printf("%u\n", count);
                }
            } else if (strcmp(argType, "d") == 0) {
                for (unsigned int i = 0; i < totalSize; i++) {
                    fread(address.bytes, 8, 1, list);//address
                    pread64(memFd, tmpData.bytes, 8, address.li);
                    if (tmpData.d > data.d) {
                        fwrite(address.bytes, 8, 1, result);
                        fwrite(tmpData.bytes, 8, 1, result);
                        currentSize++;
                    }
                    fread(tmpData.bytes, 8, 1, list);//discard value
                    count++;
//                    if (count % 100 == 0)
//                        printf("%u\n", count);
                }
            }
        } else if (strcmp(argOperation, "u") == 0) {
            if (strcmp(argType, "b") == 0) {
                for (unsigned int i = 0; i < totalSize; i++) {
                    fread(address.bytes, 8, 1, list);//address
                    fread(data.bytes, 8, 1, list);//value
                    pread64(memFd, tmpData.bytes, 1, address.li);
                    if (tmpData.c == data.c) {
                        fwrite(address.bytes, 8, 1, result);
                        fwrite(tmpData.bytes, 8, 1, result);
                        currentSize++;
                    }
                    count++;
//                    if (count % 100 == 0)
//                        printf("%u\n", count);
                }
            } else if (strcmp(argType, "s") == 0) {
                for (unsigned int i = 0; i < totalSize; i++) {
                    fread(address.bytes, 8, 1, list);//address
                    fread(data.bytes, 8, 1, list);//value
                    pread64(memFd, tmpData.bytes, 2, address.li);
                    if (tmpData.s == data.s) {
                        fwrite(address.bytes, 8, 1, result);
                        fwrite(tmpData.bytes, 8, 1, result);
                        currentSize++;
                    }
                    count++;
//                    if (count % 100 == 0)
//                        printf("%u\n", count);
                }
            } else if (strcmp(argType, "i") == 0) {
                for (unsigned int i = 0; i < totalSize; i++) {
                    fread(address.bytes, 8, 1, list);//address
                    fread(data.bytes, 8, 1, list);//value
                    pread64(memFd, tmpData.bytes, 4, address.li);
                    if (tmpData.i == data.i) {
                        fwrite(address.bytes, 8, 1, result);
                        fwrite(tmpData.bytes, 8, 1, result);
                        currentSize++;
                    }
                    count++;
//                    if (count % 100 == 0)
//                        printf("%u\n", count);
                }
            } else if (strcmp(argType, "li") == 0) {
                for (unsigned int i = 0; i < totalSize; i++) {
                    fread(address.bytes, 8, 1, list);//address
                    fread(data.bytes, 8, 1, list);//value
                    pread64(memFd, tmpData.bytes, 8, address.li);
                    if (tmpData.li == data.li) {
                        fwrite(address.bytes, 8, 1, result);
                        fwrite(tmpData.bytes, 8, 1, result);
                        currentSize++;
                    }
                    count++;
//                    if (count % 100 == 0)
//                        printf("%u\n", count);
                }
            } else if (strcmp(argType, "f") == 0) {
                for (unsigned int i = 0; i < totalSize; i++) {
                    fread(address.bytes, 8, 1, list);//address
                    fread(data.bytes, 8, 1, list);//value
                    pread64(memFd, tmpData.bytes, 4, address.li);
                    if (tmpData.f == data.f) {
                        fwrite(address.bytes, 8, 1, result);
                        fwrite(tmpData.bytes, 8, 1, result);
                        currentSize++;
                    }
                    count++;
//                    if (count % 100 == 0)
//                        printf("%u\n", count);
                }
            } else if (strcmp(argType, "d") == 0) {
                for (unsigned int i = 0; i < totalSize; i++) {
                    fread(address.bytes, 8, 1, list);//address
                    fread(data.bytes, 8, 1, list);//value
                    pread64(memFd, tmpData.bytes, 8, address.li);
                    if (tmpData.d == data.d) {
                        fwrite(address.bytes, 8, 1, result);
                        fwrite(tmpData.bytes, 8, 1, result);
                        currentSize++;
                    }
                    count++;
//                    if (count % 100 == 0)
//                        printf("%u\n", count);
                }
            }
        } else if (strcmp(argOperation, "c") == 0) {
            if (strcmp(argType, "b") == 0) {
                for (unsigned int i = 0; i < totalSize; i++) {
                    fread(address.bytes, 8, 1, list);//address
                    fread(data.bytes, 8, 1, list);//value
                    pread64(memFd, tmpData.bytes, 1, address.li);
                    if (tmpData.c != data.c) {
                        fwrite(address.bytes, 8, 1, result);
                        fwrite(tmpData.bytes, 8, 1, result);
                        currentSize++;
                    }
                    count++;
//                    if (count % 100 == 0)
//                        printf("%u\n", count);
                }
            } else if (strcmp(argType, "s") == 0) {
                for (unsigned int i = 0; i < totalSize; i++) {
                    fread(address.bytes, 8, 1, list);//address
                    fread(data.bytes, 8, 1, list);//value
                    pread64(memFd, tmpData.bytes, 2, address.li);
                    if (tmpData.s != data.s) {
                        fwrite(address.bytes, 8, 1, result);
                        fwrite(tmpData.bytes, 8, 1, result);
                        currentSize++;
                    }
                    count++;
//                    if (count % 100 == 0)
//                        printf("%u\n", count);
                }
            } else if (strcmp(argType, "i") == 0) {
                for (unsigned int i = 0; i < totalSize; i++) {
                    fread(address.bytes, 8, 1, list);//address
                    fread(data.bytes, 8, 1, list);//value
                    pread64(memFd, tmpData.bytes, 4, address.li);
                    if (tmpData.i != data.i) {
                        fwrite(address.bytes, 8, 1, result);
                        fwrite(tmpData.bytes, 8, 1, result);
                        currentSize++;
                    }
                    count++;
//                    if (count % 100 == 0)
//                        printf("%u\n", count);
                }
            } else if (strcmp(argType, "li") == 0) {
                for (unsigned int i = 0; i < totalSize; i++) {
                    fread(address.bytes, 8, 1, list);//address
                    fread(data.bytes, 8, 1, list);//value
                    pread64(memFd, tmpData.bytes, 8, address.li);
                    if (tmpData.li != data.li) {
                        fwrite(address.bytes, 8, 1, result);
                        fwrite(tmpData.bytes, 8, 1, result);
                        currentSize++;
                    }
                    count++;
//                    if (count % 100 == 0)
//                        printf("%u\n", count);
                }
            } else if (strcmp(argType, "f") == 0) {
                for (unsigned int i = 0; i < totalSize; i++) {
                    fread(address.bytes, 8, 1, list);//address
                    fread(data.bytes, 8, 1, list);//value
                    pread64(memFd, tmpData.bytes, 4, address.li);
                    if (tmpData.f != data.f) {
                        fwrite(address.bytes, 8, 1, result);
                        fwrite(tmpData.bytes, 8, 1, result);
                        currentSize++;
                    }
                    count++;
//                    if (count % 100 == 0)
//                        printf("%u\n", count);
                }
            } else if (strcmp(argType, "d") == 0) {
                for (unsigned int i = 0; i < totalSize; i++) {
                    fread(address.bytes, 8, 1, list);//address
                    fread(data.bytes, 8, 1, list);//value
                    pread64(memFd, tmpData.bytes, 8, address.li);
                    if (tmpData.d < data.d) {
                        fwrite(address.bytes, 8, 1, result);
                        fwrite(tmpData.bytes, 8, 1, result);
                        currentSize++;
                    }
                    count++;
//                    if (count % 100 == 0)
//                        printf("%u\n", count);
                }
            }
        }
        printf("%u\n", count);
        fwrite(&currentSize, sizeof(currentSize), 1, result);//totalSize
        fclose(list);
    }
    //when complete, detach from process
    ptrace(PTRACE_DETACH, pid, NULL, NULL);
    close(memFd);
    //copy result's content to list for following search, fetch 100 result to display
    list = fopen(listFilePath, "wb");
    display = fopen(displayFilePath, "w");
    generateList(result, list);
    generateDisplay(result, display, argType);
    fclose(result);
    fclose(list);
    fclose(display);
    printf("finish\n");
    return 0;
}