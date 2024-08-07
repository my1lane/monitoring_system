import socket
import threading
import time

# Словарь для хранения информации о клиентах
clients = {}

def handle_client(client_socket, client_address):
    # Информация о клиенте
    client_info = {
        'ip': client_address[0],  # IP-адрес клиента
        'port': client_address[1],  # Порт клиента
        'last_active': time.time(),  # Время последней активности клиента
        'hostname': socket.getfqdn(client_address[0])  # Имя хоста клиента
    }
    clients[client_address] = client_info

    # Открытие файла для записи скриншота от клиента
    with open(f'received_screenshot_{client_address[1]}.png', 'wb') as f:
        while True:
            data = client_socket.recv(1024)  # Получение данных от клиента
            if not data:
                break
            f.write(data)
            client_info['last_active'] = time.time()  # Обновление времени последней активности клиента
    
    client_socket.close()
    del clients[client_address]

def start_server():
    # Создание сокета для TCP соединений
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    # Привязка сокета к адресу и порту
    server.bind(('0.0.0.0', 8080))
    # Начало прослушивания входящих соединений
    server.listen(5)
    print("Server started on port 8080")

    while True:
        # Принятие вход
