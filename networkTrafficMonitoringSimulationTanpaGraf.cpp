#include <iostream> // cin cout
#include <fstream> // fungsi ofstream dan ifstream untuk logging.txt
#include <string> // ya string...
#include <cstdlib> // rand, srand untuk simulasi ping
#include <ctime> // random seed program ketika compile
#include <thread> // sleep_for untuk delay simulasi ping
#include <chrono> // std::chrono::milliseconds, delay simulasi ping 1000ms
#include <sstream> // std::stringstream, konversi string
#include <iomanip> // std::put_time, format waktu jadi string
using namespace std;

// arezyh.s

// menyimpan pesan logging
struct Log {
    string message;
    string timestamp;
    Log* next;
    Log(const string& msg, const string& time) : message(msg), timestamp(time), next(nullptr) {}
};

// menghitung penggunaan bandwidth
struct BandwidthUsage {
    string ip_address;
    int bytes_sent;
    int bytes_received;
    BandwidthUsage(const string& ip) : ip_address(ip), bytes_sent(0), bytes_received(0) {}
};

// menyimpan penggunaan bandwidth
struct BandwidthTable {
    static const int TABLE_SIZE = 10;
    BandwidthUsage* table[TABLE_SIZE];

    BandwidthTable() {
        for (int i = 0; i < TABLE_SIZE; ++i) {
            table[i] = nullptr;
        }
    }

    int hashFunction(const string& ip) {
        int hash = 0;
        for (char c : ip) {
            hash += c;
        }
        return hash % TABLE_SIZE;
    }

    void addUsage(const string& ip, int bytes, bool isSent) {
        int index = hashFunction(ip);
        if (table[index] == nullptr) {
            table[index] = new BandwidthUsage(ip);
        }
        if (isSent) {
            table[index]->bytes_sent += bytes;
        } else {
            table[index]->bytes_received += bytes;
        }
    }

    void displayUsage() {
        cout << "Bandwidth Usage:" << endl;
        for (int i = 0; i < TABLE_SIZE; ++i) {
            if (table[i] != nullptr) {
                cout << "IP: " << table[i]->ip_address << " | Sent: " << table[i]->bytes_sent << " bytes | Received: " << table[i]->bytes_received << " bytes" << endl;
            }
        }
    }
};

// Menggunakan BandwidthTable untuk melacak penggunaan bandwidth
BandwidthTable bandwidthTable;

// informasi perangkat di jaringan
struct Device {
    string ip_address;
    string status;
    string device_type;
    string gateway;
    Device(const string& ip, const string& stat, const string& type, const string& gw)
        : ip_address(ip), status(stat), device_type(type), gateway(gw) {}
};

// menyimpan informasi paket
struct Packet {
    string source_ip;
    string destination_ip;
    string message;
    Packet* next;
    Packet(const string& src, const string& dest, const string& msg = "") 
        : source_ip(src), destination_ip(dest), message(msg), next(nullptr) {}
};

// menyimpan informasi device PC dan fungsinya
struct PC {
    Device* device_info;
    string username;
    string password;
    Packet* receivedPackets;
    PC* next;
    PC* blocked_ips; // Linked list untuk IP yang diblok
    PC(const string& ip, const string& user, const string& pass, const string& gw) 
        : device_info(new Device(ip, "active", "PC", gw)), username(user), password(pass), receivedPackets(nullptr), next(nullptr), blocked_ips(nullptr) {}

    // Fungsi untuk menerima paket
    void receivePacket(Packet* packet) {
        if (device_info->status == "inactive" || isBlocked(packet->source_ip)) {
            cout << "PC dengan IP " << device_info->ip_address << " sedang mati, gateway tidak sesuai, atau IP diblok, tidak dapat menerima paket." << endl;
            return;
        }
        packet->next = receivedPackets;
        receivedPackets = packet;

        // Menambahkan penggunaan bandwidth
        bandwidthTable.addUsage(packet->source_ip, packet->message.size(), true); // Mengirim
        bandwidthTable.addUsage(packet->destination_ip, packet->message.size(), false); // Menerima
    }

    // Fungsi menampilkan paket yang diterima 
    void displayReceivedPacketsRecursive(Packet* packet) { // rekursif di sini
        if (packet == nullptr) {
            return;
        }
        cout << "Paket dari " << packet->source_ip << " ke " << packet->destination_ip << ": " << packet->message << endl;
        displayReceivedPacketsRecursive(packet->next);
    }

    void displayReceivedPackets() {
        if (!receivedPackets) {
            cout << "Tidak ada paket!" << endl;
        } else {
            displayReceivedPacketsRecursive(receivedPackets); // rekursif di sini
        }
    }

    // Fungsi untuk mematikan perangkat
    void shutdownDevice() {
        device_info->status = "inactive";
    }

    // Fungsi untuk menghidupkan perangkat
    void restartDevice() {
        device_info->status = "active";
    }

    // Konsep sorting mengurutkan paket berdasarkan IP sumber pake bubble sort.
    void sortPacketsByIP() {
        if (!receivedPackets) return;

        bool swapped;
        Packet* ptr1;
        Packet* lptr = nullptr;

        do {
            swapped = false;
            ptr1 = receivedPackets;

            while (ptr1->next != lptr) {
                if (ptr1->source_ip > ptr1->next->source_ip) {
                    swap(ptr1->source_ip, ptr1->next->source_ip);
                    swap(ptr1->destination_ip, ptr1->next->destination_ip);
                    swap(ptr1->message, ptr1->next->message);
                    swapped = true;
                }
                ptr1 = ptr1->next;
            }
            lptr = ptr1;
        } while (swapped);
    }

    // Fungsi untuk memblokir IP address
    void blockIP(const string& ip) {
        PC* newBlockedIP = new PC(ip, "", "", "");
        newBlockedIP->next = blocked_ips;
        blocked_ips = newBlockedIP;
    }

    // Fungsi untuk menghapus blokiran IP address
    void unblockIP(const string& ip) {
        PC* current = blocked_ips;
        PC* prev = nullptr;
        while (current != nullptr) {
            if (current->device_info->ip_address == ip) {
                if (prev == nullptr) {
                    blocked_ips = current->next;
                } else {
                    prev->next = current->next;
                }
                delete current;
                cout << "IP address " << ip << " berhasil dihapus dari blokiran." << endl;
                return;
            }
            prev = current;
            current = current->next;
        }
        cout << "IP address " << ip << " tidak ditemukan dalam daftar blokiran." << endl;
    }

    // Fungsi untuk memeriksa apakah IP address diblokir
    bool isBlocked(const string& ip) {
        PC* current = blocked_ips;
        while (current != nullptr) {
            if (current->device_info->ip_address == ip) {
                return true;
            }
            current = current->next;
        }
        return false;
    }

    // Fungsi untuk menampilkan IP address yang diblokir
    void displayBlockedIPs() {
        PC* current = blocked_ips;
        if (current == nullptr) {
            cout << "Tidak ada IP yang diblokir!" << endl;
            return;
        }
        while (current != nullptr) {
            cout << "IP diblokir: " << current->device_info->ip_address << endl;
            current = current->next;
        }
    }
};

// menyimpan informasi switch dan fungsinya
struct Switch {
    Device* device_info;
    PC* users[3];
    Switch* next;
    int switchIndex;
    Switch(int index, const string& ip) 
        : device_info(new Device(ip, "active", "Switch", "192.168.1.1")), next(nullptr), switchIndex(index) {
        for (int i = 0; i < 3; ++i) {
            users[i] = nullptr;
        }
    }

    void shutdownDevice() {
        device_info->status = "inactive";
        for (int i = 0; i < 3; ++i) {
            if (users[i] != nullptr) {
                users[i]->shutdownDevice();
            }
        }
    }

    void restartDevice() {
        device_info->status = "active";
        for (int i = 0; i < 3; ++i) {
            if (users[i] != nullptr) {
                users[i]->restartDevice();
            }
        }
    }
};

// menyimpan informasi tree network topologi
struct TreeNode {
    Device* device_info;
    TreeNode* children[5];
    int childCount;

    TreeNode(Device* device) : device_info(device), childCount(0) {
        for (int i = 0; i < 5; ++i) {
            children[i] = nullptr;
        }
    }
    void addChild(TreeNode* child) {
        if (childCount < 5) {
            children[childCount++] = child;
        }
    }
};

// menyimpan informasi router dan fungsinya
struct Router {
    Device* device_info;
    Switch* switches[2];
    string ip_address;
    Router(const string& ip) : device_info(new Device(ip, "active", "Router", ip)), ip_address(ip) {
        for (int i = 0; i < 2; ++i) {
            switches[i] = nullptr;
        }
    }

    void shutdownDevice() {
        device_info->status = "inactive";
        for (int i = 0; i < 2; ++i) {
            if (switches[i] != nullptr) {
                switches[i]->shutdownDevice();
            }
        }
    }

    void restartDevice() {
        device_info->status = "active";
        for (int i = 0; i < 2; ++i) {
            if (switches[i] != nullptr) {
                switches[i]->restartDevice();
            }
        }
    }
};

// konsep Queue untuk menyimpan antrian paket
struct Queue {
    Packet* front;
    Packet* rear;
    Queue() : front(nullptr), rear(nullptr) {}

    // Fungsi untuk menambahkan paket ke antrian
    void enqueue(const string& src, const string& dest, const string& msg = "") {
        Packet* newPacket = new Packet(src, dest, msg);
        if (rear == nullptr) {
            front = rear = newPacket;
            return;
        }
        rear->next = newPacket;
        rear = newPacket;
    }

    // menghapus paket dari antrian
    Packet* dequeue() {
        if (front == nullptr) {
            return nullptr;
        }
        Packet* temp = front;
        front = front->next;
        if (front == nullptr) {
            rear = nullptr;
        }
        return temp;
    }

    // memeriksa apakah antrian kosong
    bool isEmpty() {
        return front == nullptr;
    }
};

// menulis log ke file "network.txt"
void writeLogToFile(const string& message, const string& timestamp) {
    ofstream logFile("network_logs.txt", ios::app); // buka file dalam mode append
    if (logFile.is_open()) {
        logFile << "[" << timestamp << "] " << message << endl;
        logFile.close();
    } else {
        cout << "Gagal membuka file log." << endl;
    }
}

// membaca dan menampilkan log dari file langsung
void displayLogsFromFile() {
    ifstream logFile("network_logs.txt");
    if (logFile.is_open()) {
        string line;
        cout << "Logs dari file:\n";
        while (getline(logFile, line)) {
            cout << line << endl;
        }
        logFile.close();
    } else {
        cout << "Gagal membuka file log." << endl;
    }
}

// Struktur Stack untuk menyimpan log
struct Stack {
    Log* top;
    Stack() : top(nullptr) {}

    // Fungsi untuk mendapatkan waktu saat ini sebagai string
    string getCurrentTime() {
        auto now = chrono::system_clock::now();
        auto in_time_t = chrono::system_clock::to_time_t(now);
        stringstream ss;
        ss << put_time(localtime(&in_time_t), "%Y-%m-%d %X");
        return ss.str();
    }

    // Fungsi untuk menambahkan log ke stack
    void push(const string& message) {
        string currentTime = getCurrentTime();
        Log* newLog = new Log(message, currentTime);
        newLog->next = top;
        top = newLog;
        writeLogToFile(message, currentTime); // Panggil fungsi untuk menulis log ke file dengan waktu
    }

    // Fungsi untuk menghapus log dari stack
    Log* pop() {
        if (top == nullptr) {
            return nullptr;
        }
        Log* temp = top;
        top = top->next;
        return temp;
    }

    // Fungsi untuk memeriksa apakah stack kosong
    bool isEmpty() {
        return top == nullptr;
    }
};

// struktur HashNode, konsep collision handling
struct HashNode {
    string ip;
    PC* pc;
    HashNode* next;
    HashNode(const string& ip, PC* pc) : ip(ip), pc(pc), next(nullptr) {}
};

// Struktur HashTable untuk menyimpan informasi PC dengan hash table
struct HashTable {
    static const int TABLE_SIZE = 10;
    HashNode* table[TABLE_SIZE];

    HashTable() {
        for (int i = 0; i < TABLE_SIZE; ++i) {
            table[i] = nullptr;
        }
    }

    int hashFunction(const string& ip) {
        int hash = 0;
        for (char c : ip) {
            hash += c;
        }
        return hash % TABLE_SIZE;
    }

    void insert(const string& ip, PC* pc) {
        int index = hashFunction(ip);
        HashNode* newNode = new HashNode(ip, pc);
        if (table[index] == nullptr) {
            table[index] = newNode;
        } else {
            HashNode* temp = table[index];
            while (temp->next != nullptr) {
                temp = temp->next;
            }
            temp->next = newNode;
        }
    }

    // mendata router ke dalam hash table
    void insertRouter(const string& ip) {
        int index = hashFunction(ip);
        HashNode* newNode = new HashNode(ip, nullptr);
        if (table[index] == nullptr) {
            table[index] = newNode;
        } else {
            HashNode* temp = table[index];
            while (temp->next != nullptr) {
                temp = temp->next;
            }
            temp->next = newNode;
        }
    }

    void remove(const string& ip) {
        int index = hashFunction(ip);
        HashNode* temp = table[index];
        HashNode* prev = nullptr;

        while (temp != nullptr && temp->ip != ip) {
            prev = temp;
            temp = temp->next;
        }

        if (temp == nullptr) return; // IP tidak ditemukan

        if (prev == nullptr) {
            table[index] = temp->next;
        } else {
            prev->next = temp->next;
        }

        delete temp;
    }

    PC* search(const string& ip) {
        int index = hashFunction(ip);
        HashNode* temp = table[index];
        while (temp != nullptr && temp->ip != ip) {
            temp = temp->next;
        }
        return temp == nullptr ? nullptr : temp->pc;
    }

    bool searchRouter(const string& ip) {
        int index = hashFunction(ip);
        HashNode* temp = table[index];
        while (temp != nullptr) {
            if (temp->ip == ip && temp->pc == nullptr) {
                return true;
            }
            temp = temp->next;
        }
        return false;
    }

    // Fungsi untuk memperbarui IP router
    void updateRouterIP(const string& old_ip, const string& new_ip) {
        remove(old_ip);
        insertRouter(new_ip);
    }
};

// menyambungkan PC ke slot switch
void addPC(Switch* sw, int index, const string& ip_address, const string& username, const string& password, const string& gateway, const string& router_gateway, Stack& logStack, HashTable& hashTable) {
    PC* pc = new PC(ip_address, username, password, gateway);
    if (gateway != router_gateway) {
        pc->device_info->status = "inactive";
        logStack.push("Perangkat PC dengan IP " + ip_address + " terhubung ke Switch " + to_string(sw->switchIndex) + " namun gateway tidak sesuai, status menjadi inactive.");
    } else {
        pc->device_info->status = "active";
        logStack.push("Perangkat PC dengan IP " + ip_address + " terhubung ke Switch " + to_string(sw->switchIndex));
    }

    // cek apakah IP address sudah ada.
    if (hashTable.search(ip_address) != nullptr) {
        cout << "IP address " << ip_address << " sudah digunakan di jaringan. Tidak dapat menambahkan perangkat." << endl;
        logStack.push("Gagal menyambungkan PC dengan IP " + ip_address + " ke Switch " + to_string(sw->switchIndex) + ". IP address sudah digunakan.");
        return;
    }

    sw->users[index] = pc;

    // mendata ke hash table
    hashTable.insert(ip_address, pc);
    // logging
    logStack.push("Perangkat PC dengan IP " + ip_address + " terhubung ke Switch " + to_string(sw->switchIndex));
}

// Fungsi untuk memutuskan PC dari switch
void removePC(Switch* sw, int index, Stack& logStack, HashTable& hashTable) {
    if (sw->users[index] != nullptr) {
        // Tambahkan log dengan indeks switch
        logStack.push("Berhasil melepas perangkat PC " + sw->users[index]->device_info->ip_address + " dari perangkat Switch " + to_string(sw->switchIndex));
        
        // Hapus dari hash table
        hashTable.remove(sw->users[index]->device_info->ip_address);
        
        delete sw->users[index]->device_info;
        delete sw->users[index];
        sw->users[index] = nullptr;
    } else {
        cout << "PC pada slot " << index << " sedang kosong!.\n";
    }
}

// Fungsi untuk menambahkan switch ke router
void addSwitch(Router* router, int index, const string& ip, Stack& logStack) {
    Switch* sw = new Switch(index + 1, ip);
    router->switches[index] = sw;
    
    // Tambahkan log
    logStack.push("Berhasil menyambungkan perangkat Switch " + to_string(index + 1) + " ke Router " + router->ip_address + "!");
}

// Fungsi untuk menampilkan informasi PC
void displayPCs(PC* pc) {
    if (pc == nullptr) {
        return;
    }
    cout << "    PC IP: " << pc->device_info->ip_address << ", Status: " << pc->device_info->status << ", Tipe: " << pc->device_info->device_type << endl;
    displayPCs(pc->next);
}

// Fungsi untuk menampilkan informasi switch
void displaySwitches(Switch* sw, int switchIndex) {
    if (sw == nullptr) {
        return;
    }
    cout << "  Switch " << switchIndex << ", Status: " << sw->device_info->status << ", Tipe: " << sw->device_info->device_type << ":\n";
    for (int i = 0; i < 3; ++i) {
        if (sw->users[i] != nullptr) {
            cout << "    PC " << i + 1 << ": " << sw->users[i]->device_info->ip_address << endl;
        } else {
            cout << "    PC " << i + 1 << ": Tidak ada perangkat tersambung.\n";
        }
    }
    displaySwitches(sw->next, switchIndex + 1);
}

// Fungsi untuk menampilkan informasi router
void displayRouter(Router* router, int switchIndex = 1) {
    if (router == nullptr) {
        return;
    }
    cout << "Router, Status: " << router->device_info->status << ", Tipe: " << router->device_info->device_type << ", Gateway: " << router->device_info->gateway << ":\n";
    for (int i = 0; i < 2; ++i) {
        displaySwitches(router->switches[i], switchIndex + i);
    }
}

// reverse log
Log* reverseStack(Log* top) {
    Log* prev = nullptr;
    Log* current = top;
    Log* next = nullptr;

    while (current != nullptr) {
        next = current->next;
        current->next = prev;
        prev = current;
        current = next;
    }
    return prev;
}

// Fungsi untuk menampilkan log dari stack
void displayLogsStack(Stack& logStack) {
    Log* reversedTop = reverseStack(logStack.top);
    Log* temp = reversedTop;
    cout << "Logs (Runtime):\n";
    while (temp != nullptr) {
        cout << "[" << temp->timestamp << "] " << temp->message << endl;
        temp = temp->next;
    }
    reverseStack(reversedTop);  // Balik kembali ke urutan semula
}

// Konsep searching PC berdasarkan IP address yang terdaftar di jaringan
PC* findPC(Router* router, const string& ip_address, int& switchIndex, HashTable& hashTable) { // searching di sini
    PC* pc = hashTable.search(ip_address);
    if (pc != nullptr) {
        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < 3; j++) {
                if (router->switches[i] != nullptr && router->switches[i]->users[j] == pc) {
                    switchIndex = i;
                    return pc;
                }
            }
        }
    }
    return nullptr;
}

// Fungsi untuk mencari informasi IP dan memeriksa apakah itu router atau PC
void searchIP(Router* router, HashTable& hashTable) {
    string ip;
    cout << "Masukkan IP address yang ingin dicari: ";
    cin >> ip;
    int switchIndex = -1;

    if (hashTable.searchRouter(ip)) {
        cout << "IP Address : " << ip << endl;
        cout << "Status     : " << router->device_info->status << endl;
        cout << "Tipe       : " << router->device_info->device_type << endl;
        cout << "Gateway    : " << router->device_info->gateway << endl;
    } else {
        PC* pc = findPC(router, ip, switchIndex, hashTable);
        if (pc != nullptr) {
            cout << "IP Address : " << pc->device_info->ip_address << endl;
            cout << "Status     : " << pc->device_info->status << endl;
            cout << "Tipe       : " << pc->device_info->device_type << endl;
            cout << "Username   : " << pc->username << endl;
            cout << "Password   : " << pc->password << endl;
            cout << "Gateway    : " << pc->device_info->gateway << endl;
            cout << "Terkoneksi dengan Switch: " << switchIndex + 1 << endl;
            cout << "Terkoneksi dengan Router: " << router->ip_address << endl;
            cout << "Paket yang diterima:" << endl;
            pc->displayReceivedPackets();
            cout << "IP yang diblokir:" << endl;
            pc->displayBlockedIPs();
        } else {
            cout << "Tidak ada perangkat dengan IP address " << ip << endl;
        }
    }
}


// Fungsi untuk memproses paket
void processPackets(Queue& packetQueue, Stack& logStack, Router* router, HashTable& hashTable) {
    while (!packetQueue.isEmpty()) {
        Packet* packet = packetQueue.dequeue();
        if (packet != nullptr) {
            if (packet->message.find("DDoS Packet") != string::npos) {
                cout << "Mendeteksi paket DDoS dari " << packet->source_ip << " ke " << packet->destination_ip << endl;
                logStack.push("Mendeteksi paket DDoS dari " + packet->source_ip + " ke " + packet->destination_ip);
            } else {
                cout << "Memproses paket dari " << packet->source_ip << " ke " << packet->destination_ip << ". Terima atau tolak (Y/n)? ";
                char decision;
                cin >> decision;
                if (decision == 'Y' || decision == 'y') {
                    int switchIndex = -1;
                    PC* destPC = findPC(router, packet->destination_ip, switchIndex, hashTable);
                    if (destPC && destPC->device_info->status == "active" && !destPC->isBlocked(packet->source_ip)) {
                        destPC->receivePacket(packet);
                        bandwidthTable.addUsage(packet->source_ip, packet->message.size(), true);
                        bandwidthTable.addUsage(packet->destination_ip, packet->message.size(), false);
                        logStack.push("Paket dari " + packet->source_ip + " ke " + packet->destination_ip + " diterima.");
                    } else {
                        logStack.push("Paket dari " + packet->source_ip + " ke " + packet->destination_ip + " ditolak (atau destinasi tidak ditemukan).");
                        delete packet;
                    }
                } else {
                    logStack.push("Paket dari " + packet->source_ip + " ke " + packet->destination_ip + " ditolak.");
                    delete packet;
                }
            }
        }
    }
}

// Fungsi untuk melakukan ping
void pingRouter(const string& source_ip, Router* router, Stack& logStack, HashTable& hashTable) {
    int switchIndex = -1;
    PC* pc = findPC(router, source_ip, switchIndex, hashTable);
    if (pc == nullptr || pc->device_info->status == "inactive" || pc->device_info->gateway != router->ip_address) {
        cout << "PC dengan IP " << source_ip << " tidak terhubung atau gateway tidak sesuai, tidak dapat melakukan ping ke router." << endl;
        logStack.push("Ping dari IP " + source_ip + " ke router " + router->ip_address + " gagal, PC tidak terhubung atau gateway tidak sesuai.");
        return;
    }

    srand(time(0));
    cout << "Pinging router " << router->ip_address << " from IP " << source_ip << " with 32 bytes of data:" << endl;
    logStack.push("Pinging router " + router->ip_address + " from IP " + source_ip);
    
    int times[4];  // Array untuk menyimpan waktu round-trip setiap ping
    
    for (int i = 0; i < 4; ++i) {
        int time = rand() % 100 + 1;  // Generate random round-trip time between 1ms and 100ms
        times[i] = time;  // Simpan waktu ping ke array
        this_thread::sleep_for(chrono::milliseconds(1000));  // Simulasi delay ping
        cout << "Reply from " << router->ip_address << + ": bytes=32 time=" << time << "ms TTL=64" << endl;
    }
    
    // Hitung minimum, maximum, dan average
    int minTime = times[0];
    int maxTime = times[0];
    int sumTime = 0;
    
    for (int i = 0; i < 4; ++i) {
        if (times[i] < minTime) {
            minTime = times[i];
        }
        if (times[i] > maxTime) {
            maxTime = times[i];
        }
        sumTime += times[i];
    }
    
    int avgTime = sumTime / 4;
    
    cout << endl << "Ping statistics for router " << router->ip_address << ":" << endl;
    cout << "    Packets: Sent = 4, Received = 4, Lost = 0 (0% loss)," << endl;
    cout << "Approximate round trip times in milli-seconds:" << endl;
    cout << "    Minimum = " << minTime << "ms, Maximum = " << maxTime << "ms, Average = " << avgTime << "ms" << endl;
    logStack.push("Ping statistics for router " + router->ip_address + " from IP " + source_ip);
    logStack.push("    Minimum = " + to_string(minTime) + "ms, Maximum = " + to_string(maxTime) + "ms, Average = " + to_string(avgTime) + "ms");
}

// ubah IP address perangkat
void changeDeviceIP(HashTable& hashTable, PC* pc, const string& new_ip, Stack& logStack) {
    if (hashTable.search(new_ip) != nullptr) {
        cout << "IP address " << new_ip << " sudah digunakan di jaringan. Tidak dapat mengubah IP address." << endl;
        logStack.push("Gagal mengubah IP address PC dengan IP " + pc->device_info->ip_address + " menjadi " + new_ip + ". IP address sudah digunakan.");
        return;
    }
    string old_ip = pc->device_info->ip_address;
    hashTable.remove(old_ip);
    pc->device_info->ip_address = new_ip;
    hashTable.insert(new_ip, pc);
    logStack.push("IP address PC dengan IP " + old_ip + " berhasil diubah menjadi " + new_ip);
}

// ubah IP address gateway
void changeDeviceGateway(PC* pc, const string& new_gateway, const string& router_gateway, Stack& logStack) {
    string old_gateway = pc->device_info->gateway;
    pc->device_info->gateway = new_gateway;
    if (new_gateway != router_gateway) {
        pc->device_info->status = "inactive";
        logStack.push("Gateway PC dengan IP " + pc->device_info->ip_address + " berhasil diubah dari " + old_gateway + " menjadi " + new_gateway + ", namun status menjadi inactive karena gateway tidak sesuai.");
    } else {
        pc->device_info->status = "active";
        logStack.push("Gateway PC dengan IP " + pc->device_info->ip_address + " berhasil diubah dari " + old_gateway + " menjadi " + new_gateway + ".");
    }
}

// Fungsi untuk mengubah IP address router dan memperbarui gateway perangkat yang terhubung
void changeRouterIP(Router* router, const string& new_ip, HashTable& hashTable, Stack& logStack) {
    string old_ip = router->ip_address;
    router->device_info->ip_address = new_ip;
    router->ip_address = new_ip;
    router->device_info->gateway = new_ip;
    logStack.push("IP address router diubah dari " + old_ip + " menjadi " + new_ip);

    // Perbarui hash table
    hashTable.updateRouterIP(old_ip, new_ip);

    // Periksa dan perbarui status perangkat lain
    for (int i = 0; i < 2; ++i) {
        if (router->switches[i] != nullptr) {
            for (int j = 0; j < 3; ++j) {
                if (router->switches[i]->users[j] != nullptr) {
                    PC* pc = router->switches[i]->users[j];
                    if (pc->device_info->gateway != new_ip) {
                        pc->device_info->status = "inactive";
                        logStack.push("PC dengan IP " + pc->device_info->ip_address + " menjadi inactive karena gateway tidak sesuai dengan IP router baru.");
                    } else {
                        pc->device_info->status = "active";
                    }
                }
            }
        }
    }
}

// menampilkan tree topology network
void displayTree(TreeNode* node, int level = 0) {
    if (node == nullptr) return;
    for (int i = 0; i < level; ++i) cout << "  ";
    cout << node->device_info->device_type << " IP: " << node->device_info->ip_address << " Status: " << node->device_info->status << endl;
    for (int i = 0; i < node->childCount; ++i) {
        displayTree(node->children[i], level + 1);
    }
}

// update tree topology network
void updateTree(TreeNode* root, Router* router) {
    root->childCount = 0; // Reset children count
    for (int i = 0; i < 2; ++i) {
        if (router->switches[i] != nullptr) {
            TreeNode* switchNode = new TreeNode(router->switches[i]->device_info);
            root->addChild(switchNode);
            for (int j = 0; j < 3; ++j) {
                if (router->switches[i]->users[j] != nullptr) {
                    TreeNode* pcNode = new TreeNode(router->switches[i]->users[j]->device_info);
                    switchNode->addChild(pcNode);
                }
            }
        }
    }
}

// Fungsi untuk simulasi serangan DDoS
void simulateDDoSAttack(Router* router, const string& target_ip, int packetCount, Queue& packetQueue, Stack& logStack, HashTable& hashTable) {
    int switchIndex = -1;
    PC* targetPC = findPC(router, target_ip, switchIndex, hashTable);
    
    if (!targetPC || targetPC->device_info->status == "inactive") {
        cout << "PC dengan IP " << target_ip << " tidak ditemukan atau sedang mati." << endl;
        logStack.push("Simulasi serangan DDoS gagal: PC dengan IP " + target_ip + " tidak ditemukan atau sedang mati.");
        return;
    }

    logStack.push("Mulai simulasi serangan DDoS ke IP " + target_ip);

    for (int i = 0; i < packetCount; ++i) {
        string source_ip = "192.168.1." + to_string((i % 10) + 20);  // Multiple sources
        Packet* packet = new Packet(source_ip, target_ip, "DDoS Packet " + to_string(i + 1));
        packetQueue.enqueue(packet->source_ip, packet->destination_ip, packet->message);

        // Tambahkan penggunaan bandwidth untuk sumber dan tujuan
        bandwidthTable.addUsage(source_ip, packet->message.size(), true); // Mengirim
        bandwidthTable.addUsage(target_ip, packet->message.size(), false); // Menerima

        // Jika PC target masih aktif, tambahkan paket DDoS ke PC target
        if (targetPC->device_info->status == "active") {
            targetPC->receivePacket(packet);
        }
    }

    logStack.push("Simulasi serangan DDoS ke IP " + target_ip + " dengan " + to_string(packetCount) + " paket selesai.");
}

// Fungsi untuk mitigasi serangan DDoS
void mitigateDDoSAttack(Router* router, const string& target_ip, Stack& logStack, HashTable& hashTable) {
    if (router->device_info->ip_address == target_ip) {
        if (router->device_info->status == "active") {
            router->shutdownDevice();
            logStack.push("Mitigasi serangan DDoS: Router dengan IP " + target_ip + " dimatikan untuk mencegah kerusakan lebih lanjut.");
            cout << "Router dengan IP " << target_ip << " berhasil dimatikan." << endl;
        } else {
            logStack.push("Mitigasi serangan DDoS gagal: Router dengan IP " + target_ip + " sudah mati.");
            cout << "Router dengan IP " << target_ip << " sudah mati." << endl;
        }
    } else {
        int switchIndex = -1;
        PC* targetPC = findPC(router, target_ip, switchIndex, hashTable);
        if (targetPC && targetPC->device_info->status == "active") {
            targetPC->shutdownDevice();
            logStack.push("Mitigasi serangan DDoS: PC dengan IP " + target_ip + " dimatikan untuk mencegah kerusakan lebih lanjut.");
            cout << "PC dengan IP " + target_ip << " berhasil dimatikan." << endl;
        } else {
            logStack.push("Mitigasi serangan DDoS gagal: PC dengan IP " + target_ip + " tidak ditemukan atau sudah mati.");
            cout << "PC dengan IP " + target_ip + " tidak ditemukan atau sudah mati." << endl;
        }
    }

    // Matikan perangkat sumber serangan
    for (int i = 0; i < 10; ++i) {
        string source_ip = "192.168.1." + to_string(i + 20);
        int sourceSwitchIndex = -1;
        PC* sourcePC = findPC(router, source_ip, sourceSwitchIndex, hashTable);
        if (sourcePC && sourcePC->device_info->status == "active") {
            sourcePC->shutdownDevice();
            logStack.push("Sumber serangan DDoS: PC dengan IP " + source_ip + " dimatikan.");
            cout << "PC dengan IP " + source_ip << " berhasil dimatikan." << endl;
        }
    }
}

// Fungsi untuk menyalakan perangkat
void restartDevice(HashTable& hashTable, Router* router, const string& ip, Stack& logStack) {
    if (router->device_info->ip_address == ip) {
        router->restartDevice();
        logStack.push("Perangkat router dengan IP " + ip + " berhasil dinyalakan.");
        cout << "Router dengan IP " + ip + " berhasil dinyalakan." << endl;
    } else {
        int switchIndex = -1;
        PC* pc = findPC(router, ip, switchIndex, hashTable);
        if (pc != nullptr) {
            pc->restartDevice();
            logStack.push("Perangkat PC dengan IP " + ip + " berhasil dinyalakan.");
            cout << "PC dengan IP " + ip + " berhasil dinyalakan." << endl;
        } else {
            logStack.push("Perangkat dengan IP " + ip + " tidak ditemukan.");
            cout << "Perangkat dengan IP " + ip + " tidak ditemukan." << endl;
        }
    }
}

void shutdownDevice(HashTable& hashTable, Router* router, const string& ip, Stack& logStack) {
    if (router->device_info->ip_address == ip) {
        router->shutdownDevice();
        logStack.push("Perangkat router dengan IP " + ip + " berhasil dimatikan.");
        cout << "Router dengan IP " + ip + " berhasil dimatikan." << endl;
    } else {
        int switchIndex = -1;
        PC* pc = findPC(router, ip, switchIndex, hashTable);
        if (pc != nullptr) {
            pc->shutdownDevice();
            logStack.push("Perangkat PC dengan IP " + ip + " berhasil dimatikan.");
            cout << "PC dengan IP " + ip + " berhasil dimatikan." << endl;
        } else {
            logStack.push("Perangkat dengan IP " + ip + " tidak ditemukan.");
            cout << "Perangkat dengan IP " + ip + " tidak ditemukan." << endl;
        }
    }
}


// Fungsi untuk memblokir IP address
void blockIPAddress(PC* pc) {
    string ip;
    cout << "Masukkan IP address yang ingin diblokir: ";
    cin >> ip;
    pc->blockIP(ip);
    cout << "IP address " << ip << " berhasil diblokir." << endl;
}

// Fungsi untuk menghapus blokiran IP address
void unblockIPAddress(PC* pc) {
    string ip;
    cout << "Masukkan IP address yang ingin dihapus dari blokiran: ";
    cin >> ip;
    pc->unblockIP(ip);
}

// Fungsi untuk menampilkan IP address yang diblokir
void displayBlockedIPs(PC* pc) {
    cout << "IP address yang diblokir:" << endl;
    pc->displayBlockedIPs();
}

// Menu untuk simulasi jaringan sebagai administrator jaringan
bool adminMenu(Router* router, Stack& logStack, Queue& packetQueue, HashTable& hashTable, TreeNode* root) {
    int choice;
    do {
        system("cls");
        cout << "   Simulasi Administrator Jaringan\n";
        cout << "1. Konfigurasi" << endl;
        cout << "2. Monitoring" << endl;
        cout << "3. Proses Paket" << endl;
        cout << "4. Mode Simulasi" << endl;
        cout << "0. Logout" << endl;
        cout << ">> ";
        cin >> choice;

        switch(choice) {
            case 1: { // Konfigurasi
                int pilihKonfigurasi;
                do {
                    system("cls");
                    cout << "   Konfigurasi Perangkat dan Jaringan\n";
                    cout << "1. Tambahkan Device PC baru ke Jaringan\n";
                    cout << "2. Menghapus Koneksi PC dari Perangkat Switch\n";
                    cout << "3. Ubah Konfigurasi Jaringan\n";
                    cout << "4. Nyalakan/Mematikan Perangkat\n";
                    cout << "0. Kembali\n";
                    cout << ">> ";
                    cin >> pilihKonfigurasi;

                    switch(pilihKonfigurasi) {
                        case 1: {
                            int switchIndex, pcIndex;
                            string ip, username, password, gateway;
                            cout << "Switch (0 atau 1): ";
                            cin >> switchIndex;
                            cout << "Pilih PC Index (0 atau 2): ";
                            cin >> pcIndex;
                            cout << "Masukkan PC IP Address: ";
                            cin >> ip;
                            cout << "Masukkan Username: ";
                            cin >> username;
                            cout << "Masukkan Password: ";
                            cin >> password;
                            cout << "Masukkan Gateway: ";
                            cin >> gateway;
                            if (router->switches[switchIndex] != nullptr && router->switches[switchIndex]->users[pcIndex] == nullptr) {
                                addPC(router->switches[switchIndex], pcIndex, ip, username, password, gateway, router->ip_address, logStack, hashTable);
                            } else {
                                cout << "Invalid Switch/PC Index atau sudah ada PC yang tersambung dengan socket.\n";
                            }
                            updateTree(root, router);
                            system("pause");
                            break;
                        }
                        case 2: {
                            int switchIndex, pcIndex;
                            cout << "Pilih Switch Index (0 or 1): ";
                            cin >> switchIndex;
                            cout << "Pilih PC Index (0 to 2): ";
                            cin >> pcIndex;
                            if (router->switches[switchIndex] != nullptr) {
                                removePC(router->switches[switchIndex], pcIndex, logStack, hashTable);
                            } else {
                                cout << "Invalid Switch Index.\n";
                            }
                            updateTree(root, router);
                            system("pause");
                            break;
                        }
                        case 3: {
                            int type;
                            cout << "Pilih perangkat untuk dikonfigurasi:\n1. PC\n2. Switch\n3. Router\n>> ";
                            cin >> type;
                            if (type == 1) {
                                string ip;
                                cout << "Masukkan IP address perangkat PC: ";
                                cin >> ip;
                                int switchIndex = -1;
                                PC* pc = findPC(router, ip, switchIndex, hashTable);
                                if (pc != nullptr) {
                                    int pilihKonfigurasi;
                                    cout << "Pilih konfigurasi untuk diubah:\n1. IP Address\n2. Gateway\n>> ";
                                    cin >> pilihKonfigurasi;
                                    if (pilihKonfigurasi == 1) {
                                        string new_ip;
                                        cout << "Masukkan IP Address baru: ";
                                        cin >> new_ip;
                                        changeDeviceIP(hashTable, pc, new_ip, logStack);
                                    } else if (pilihKonfigurasi == 2) {
                                        string new_gateway;
                                        cout << "Masukkan Gateway baru: ";
                                        cin >> new_gateway;
                                        changeDeviceGateway(pc, new_gateway, router->ip_address, logStack);
                                    } else {
                                        cout << "Pilihan tidak valid.\n";
                                    }
                                } else {
                                    cout << "PC tidak ditemukan.\n";
                                }
                            } else if (type == 2) {
                                string ip;
                                cout << "Masukkan IP address perangkat Switch: ";
                                cin >> ip;
                                int switchIndex = -1;
                                for (int i = 0; i < 2; ++i) {
                                    if (router->switches[i] != nullptr && router->switches[i]->device_info->ip_address == ip) {
                                        switchIndex = i;
                                        break;
                                    }
                                }
                                if (switchIndex != -1) {
                                    cout << "Pilih konfigurasi untuk diubah:\n1. IP Address\n>> ";
                                    int pilihKonfigurasi;
                                    cin >> pilihKonfigurasi;
                                    if (pilihKonfigurasi == 1) {
                                        string new_ip;
                                        cout << "Masukkan IP Address baru: ";
                                        cin >> new_ip;
                                        router->switches[switchIndex]->device_info->ip_address = new_ip;
                                        logStack.push("IP address Switch dengan IP " + ip + " berhasil diubah menjadi " + new_ip);
                                    } else {
                                        cout << "Pilihan tidak valid.\n";
                                    }
                                } else {
                                    cout << "Switch tidak ditemukan.\n";
                                }
                            } else if (type == 3) {
                                cout << "Pilih konfigurasi untuk diubah:\n1. IP Address\n>> ";
                                int pilihKonfigurasi;
                                cin >> pilihKonfigurasi;
                                if (pilihKonfigurasi == 1) {
                                    string new_ip;
                                    cout << "Masukkan IP Address baru: ";
                                    cin >> new_ip;
                                    changeRouterIP(router, new_ip, hashTable, logStack);
                                } else {
                                    cout << "Pilihan tidak valid.\n";
                                }
                            } else {
                                cout << "Pilihan tidak valid.\n";
                            }
                            updateTree(root, router);
                            system("pause");
                            break;
                        }
                        case 4: {
                            string ip;
                            cout << "Masukkan IP address perangkat yang ingin dinyalakan/dimatikan: ";
                            cin >> ip;
                            cout << "Pilih mode:\n1. Hidup\n2. Shutdown\n>> ";
                            int action;
                            cin >> action;
                            if (action == 1) {
                                restartDevice(hashTable, router, ip, logStack);
                            } else if (action == 2) {
                                shutdownDevice(hashTable, router, ip, logStack);
                            } else {
                                cout << "Pilihan tidak valid.\n";
                            }
                            system("pause");
                            break;
                        }
                        case 0:
                            break;
                        default:
                            cout << "Pilihan tidak valid. Coba lagi.\n";
                    }
                } while (pilihKonfigurasi != 0);
                break;
            }
            case 2: { // Monitoring jaringan
                int pilihMonitoring;
                do {
                    system("cls");
                    cout << "   Monitoring Jaringan\n";
                    cout << "1. Tampilkan Topologi Jaringan\n";
                    cout << "2. Tampilkan Log(s) Jaringan\n";
                    cout << "3. Cari IP Terdaftar\n";
                    cout << "4. Tampilkan Penggunaan Bandwidth\n";
                    cout << "0. Kembali\n";
                    cout << ">> ";
                    cin >> pilihMonitoring;

                    switch(pilihMonitoring) {
                        case 1:
                            displayTree(root);
                            system("pause");
                            break;
                        case 2:
                            displayLogsStack(logStack);
                            system("pause");
                            break;
                        case 3:
                            searchIP(router, hashTable);
                            system("pause");
                            break;
                        case 4:
                            bandwidthTable.displayUsage();
                            system("pause");
                            break;
                        case 0:
                            break;
                        default:
                            cout << "Pilihan tidak valid. Coba lagi.\n";
                    }
                } while (pilihMonitoring != 0);
                break;
            }
            case 3: { // Proses Paket
                system("cls");
                processPackets(packetQueue, logStack, router, hashTable);
                system("pause");
                break;
            }
            case 4: { // Simulasi
                int pilihSimulasi;
                do {
                    system("cls");
                    cout << "   Simulasi Jaringan\n";
                    cout << "1. Simulasi Serangan DDoS\n";
                    cout << "2. Mitigasi Serangan DDoS\n";
                    cout << "0. Kembali\n";
                    cout << ">> ";
                    cin >> pilihSimulasi;

                    switch(pilihSimulasi) {
                        case 1: {
                            string target_ip;
                            int packetCount;
                            cout << "Masukkan IP address target: ";
                            cin >> target_ip;
                            cout << "Masukkan jumlah paket: ";
                            cin >> packetCount;
                            simulateDDoSAttack(router, target_ip, packetCount, packetQueue, logStack, hashTable);
                            system("pause");
                            break;
                        }
                        case 2: {
                            string target_ip;
                            cout << "Masukkan IP address target yang ingin dimitigasi: ";
                            cin >> target_ip;
                            mitigateDDoSAttack(router, target_ip, logStack, hashTable);
                            system("pause");
                            break;
                        }
                        case 0:
                            break;
                        default:
                            cout << "Pilihan tidak valid. Coba lagi.\n";
                    }
                } while (pilihSimulasi != 0);
                break;
            }
            case 0:
                return false;
            default:
                cout << "Pilihan tidak valid. Coba lagi.\n";
        }
    } while (choice != 0);
    return true;
}

// Menu simulasi jaringan sebagai user
bool userMenu(Router* router, Queue& packetQueue, Stack& logStack, HashTable& hashTable) {
    system("cls");
    string ip, username, password;
    cout << "Masukkan IP address perangkat PC anda: ";
    cin >> ip;
    int switchIndex = -1;
    PC* pc = findPC(router, ip, switchIndex, hashTable);
    if (pc != nullptr) {
        cout << "Username: ";
        cin >> username;
        cout << "Password: ";
        cin >> password;
        if (pc->username == username && pc->password == password) {
            logStack.push("User " + username + " logged in from IP " + ip);
            int choice;
            do {
                system("cls");
                cout << "   Simulasi Pengguna Jaringan " << endl;
                cout << "Logged in as : " + pc->username << endl;
                cout << "Status       : " + pc->device_info->status << endl;
                cout << "IP Address   : " + pc->device_info->ip_address << endl;
                cout << "1. Kirim Paket ke PC Lain" << endl;
                cout << "2. Lihat Paket dari PC Lain" << endl;
                cout << "3. Ping Router" << endl;
                cout << "4. Tampilkan Penggunaan Bandwidth" << endl;
                cout << "5. Matikan atau Nyalakan Ulang Perangkat" << endl;
                cout << "6. Blokir IP Address" << endl;
                cout << "7. Hapus Blokiran IP Address" << endl;
                cout << "8. Tampilkan IP Address yang Diblokir" << endl;
                cout << "0. Logout" << endl;
                cout << ">> ";
                cin >> choice;

                switch (choice) {
                    case 1: {
                        string dest_ip, message;
                        cout << "Masukkan destinasi IP: ";
                        cin >> dest_ip;
                        cout << "Masukkan pesan: ";
                        cin.ignore();
                        getline(cin, message);
                        packetQueue.enqueue(ip, dest_ip, message);
                        cout << "Paket anda dari " << ip << " ke " << dest_ip << " sedang dikirim!" << endl;
                        cout << "Silahkan tunggu admin untuk mengonfirmasi paket Anda! :))" << endl;
                        logStack.push("User " + username + " mengirim paket dari IP " + ip + " ke IP " + dest_ip + " dengan pesan: " + message);
                        system("pause");
                        break;
                    }
                    case 2:
                        cout << "Paket yang diterima:" << endl;
                        pc->sortPacketsByIP();
                        pc->displayReceivedPackets();
                        system("pause");
                        break;
                    case 3:
                        if (pc->device_info->status == "inactive") {
                            cout << "PC anda sedang mati, tidak dapat melakukan ping ke router." << endl;
                            logStack.push("User " + username + " dengan IP " + ip + " gagal melakukan ping ke router karena PC mati.");
                        } else {
                            pingRouter(ip, router, logStack, hashTable);
                        }
                        system("pause");
                        break;
                    case 4:
                        cout << "Penggunaan Bandwidth Anda:" << endl;
                        for (int i = 0; i < BandwidthTable::TABLE_SIZE; ++i) {
                            if (bandwidthTable.table[i] != nullptr && bandwidthTable.table[i]->ip_address == ip) {
                                cout << "IP: " << bandwidthTable.table[i]->ip_address << " | Sent: " << bandwidthTable.table[i]->bytes_sent << " bytes | Received: " << bandwidthTable.table[i]->bytes_received << " bytes" << endl;
                            }
                        }
                        system("pause");
                        break;
                    case 5: {
                        cout << "Pilih tindakan:\n1. Matikan\n2. Nyalakan ulang\n>> ";
                        int action;
                        cin >> action;
                        if (action == 1) {
                            pc->shutdownDevice();
                            logStack.push("User " + username + " mematikan perangkat dengan IP " + ip);
                            cout << "Perangkat dengan IP " << ip << " berhasil dimatikan." << endl;
                        } else if (action == 2) {
                            pc->restartDevice();
                            logStack.push("User " + username + " menyalakan ulang perangkat dengan IP " + ip);
                            cout << "Perangkat dengan IP " << ip << " berhasil dinyalakan ulang." << endl;
                        } else {
                            cout << "Pilihan tidak valid.\n";
                        }
                        system("pause");
                        break;
                    }
                    case 6: {
                        blockIPAddress(pc);
                        logStack.push("User " + username + " memblokir IP address dari IP " + ip);
                        system("pause");
                        break;
                    }
                    case 7: {
                        unblockIPAddress(pc);
                        logStack.push("User " + username + " menghapus blokiran IP address dari IP " + ip);
                        system("pause");
                        break;
                    }
                    case 8: {
                        displayBlockedIPs(pc);
                        system("pause");
                        break;
                    }
                    case 0:
                        logStack.push("User " + username + " logged out from IP " + ip);
                        return false;
                    default:
                        cout << "Input tidak valid!\n";
                }
            } while (choice != 0);
        } else {
            cout << "Password atau kata sandi anda salah!\n";
            logStack.push("Login gagal untuk user " + username + " dengan IP " + ip + " karena password salah.");
            system("pause");
        }
    } else {
        cout << "Perangkat PC dengan IP address " << ip << " tidak ditemukan.\n";
        logStack.push("Login gagal untuk user " + username + " dengan IP " + ip + ". Perangkat tidak ditemukan.");
        system("pause");
    }
    return true;
}

// int main program
int main() {
    Router* router = new Router("192.168.1.1"); // IP address router
    Stack logStack;
    Queue packetQueue;
    HashTable hashTable;

    // Tambahkan router ke hash table
    hashTable.insertRouter(router->ip_address);

    // inisialiasi switch 1 dan 2 langsung
    addSwitch(router, 0, "192.168.1.100", logStack);
    addSwitch(router, 1, "192.168.2.100", logStack);

    // tambah pc ke switch -> router // switch 1
    addPC(router->switches[0], 0, "192.168.1.2", "user1", "pass1", "192.168.1.1", router->ip_address, logStack, hashTable);
    addPC(router->switches[0], 1, "192.168.1.3", "user2", "pass2", "192.168.1.1", router->ip_address, logStack, hashTable);
    addPC(router->switches[0], 2, "192.168.1.4", "user3", "pass3", "192.168.1.1", router->ip_address, logStack, hashTable);
    // switch 2
    addPC(router->switches[1], 0, "192.168.2.5", "user4", "pass4", "192.168.1.1", router->ip_address, logStack, hashTable);
    addPC(router->switches[1], 1, "192.168.2.6", "user5", "pass5", "192.168.1.1", router->ip_address, logStack, hashTable);
    addPC(router->switches[1], 2, "192.168.2.7", "user6", "pass6", "192.168.1.1", router->ip_address, logStack, hashTable);

    TreeNode* root = new TreeNode(router->device_info);
    updateTree(root, router);

    while (true) {
        int userType;
        system("cls");
        cout << "       Network Traffic Monitoring &      " << endl;
        cout << "   Simulation of Tree Topology Network   " << endl;
        cout << endl;
        cout << "Mode simulasi jaringan: " << endl;
        cout << "1. Administrator Jaringan  " << endl;
        cout << "2. Pengguna Jaringan       " << endl;
        cout << "3. Tampilkan Tree Topology\n";
        cout << "4. Keluar" << endl;
        cout << ">> ";
        cin >> userType;
        if (userType == 1) {
            system("cls");
            if (!adminMenu(router, logStack, packetQueue, hashTable, root)) { // menu admin
                continue;  // ini untuk kembali ke menu login
            }
        } else if (userType == 2) {
            userMenu(router, packetQueue, logStack, hashTable); // menu user
        } else if (userType == 3) {
            displayTree(root); // tree topologi
            system("pause");
        } else if (userType == 4) {
            break;
        } else {
            cout << "Salah!" << endl;
        }
    }

    // hapus memori
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 3; j++) {
            if (router->switches[i]->users[j] != nullptr) {
                delete router->switches[i]->users[j]->device_info;
                delete router->switches[i]->users[j];
            }
        }
        delete router->switches[i]->device_info;
        delete router->switches[i];
    }
    delete router->device_info;
    delete router;

    while (logStack.top != nullptr) {
        Log* temp = logStack.pop();
        delete temp;
    }

    while (packetQueue.front != nullptr) {
        Packet* temp = packetQueue.dequeue();
        delete temp;
    }

    return 0;
}