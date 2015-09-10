Bintang Messenger
==========
![Bintang Messenger](/../screenshoot/screenshoots/login.jpg?raw=true "Bintang Messenger")

Bintang Messenger adalah sebuah aplikasi chat sederhana berbasis client-server. Bintang messenger dibuat dengan menggunakan Unix Socket dan C++ 11.

## Protokol
Protokol yang digunakan oleh Bintang Messenger dapat dilihat pada `Desain Protokol.docx`.

## Implementasi
### Server
Setelah dijalankan server melakukan LISTEN di port 8080 dan menunggu koneksi dari klien. Ketika klien melakukan koneksi, server akan membuat thread baru dan meng-accept koneksi tersebut di socket baru yang nantinya akan diproses pada thread yang baru tersebut. Sehingga jumlah thread yang terbentuk sesuai dengan klien yang terhubung ditambah satu thread untuk listen. Di thread yang terbentuk akibat koneksi dari klien, server akan melakukan receive secara terus menerus sampai klien menyatakan logout/disconnect dan memproses apa yang dikirim oleh klien. Server menyimpan seluruh data klien yang terkoneksi dalam memory dengan struktur data datakoneksi dan akan menghapusnya ketika klien logout atau disconnect.

Jika server harus mem-forward data dari satu klien ke klien lainnya, misalnya saat mengirimkan pesan, server akan terlebih dahulu mencari user tujuan yang sedang online di memory. Jika user tujuan tidak ada, maka server akan menyimpan data tersebut di penyimpanan offline.

## Client
Klien akan melakukan koneksi ke alamat yang sudah didefinisikan. Pada program yang kami buat adalah ke localhost dengan port 8080. Dalam mengimplementasikan klien, kami menggunakan dua buah thread agar proses penerimaan data dapat dilakukan walaupun thread utama sedang di-block oleh input user.

Thread utama akan menangani setiap perintah dari user dan memprosesnya. Misalnya perintah untuk mengirimkan pesan ke pengguna tertentu. Thread ke-dua digunakan untuk menerima data dari server. Agar kedua thread ini tidak saling konflik, kami menggunakan sistem mutual exclusive untuk beberapa kasus tertentu seperti kasus pengiriman pesan (karena harus menunggu konfirmasi dari server).

## Screen Shoots
Server logging:

![Server](/../screenshoot/screenshoots/server.jpg?raw=true "Bintang Messenger")

Client dapat melakukan login atau pendaftaran akun:

![Login](/../screenshoot/screenshoots/login.jpg?raw=true "Bintang Messenger")

List Inbox Client:

![Inbox](/../screenshoot/screenshoots/list.jpg?raw=true "Bintang Messenger")

Tampilan pesan (modus thread):

![Read Message](/../screenshoot/screenshoots/readmessage.jpg?raw=true "Bintang Messenger")

## Lisensi
MIT License
