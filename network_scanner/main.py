import tkinter as tk
import threading
import socket
import requests
from scapy.all import *

class PortScannerGUI:
    def __init__(self, master):
        self.master = master
        master.title("Port Scanner")

        self.create_widgets()

    def create_widgets(self):
        self.create_input_fields()
        self.create_scan_buttons()
        self.create_result_widgets()

    def create_input_fields(self):
        self.label_target = tk.Label(self.master, text="Target Hostname:")
        self.label_target.grid(row=0, column=0, sticky="w", padx=10, pady=5)

        self.entry_target = tk.Entry(self.master)
        self.entry_target.grid(row=0, column=1, padx=10, pady=5)

        self.label_start_port = tk.Label(self.master, text="Start Port:")
        self.label_start_port.grid(row=1, column=0, sticky="w", padx=10, pady=5)

        self.entry_start_port = tk.Entry(self.master)
        self.entry_start_port.grid(row=1, column=1, padx=10, pady=5)

        self.label_end_port = tk.Label(self.master, text="End Port:")
        self.label_end_port.grid(row=2, column=0, sticky="w", padx=10, pady=5)

        self.entry_end_port = tk.Entry(self.master)
        self.entry_end_port.grid(row=2, column=1, padx=10, pady=5)

    def create_scan_buttons(self):
        self.scan_ports_button = tk.Button(self.master, text="Scan Ports", command=self.scan_ports)
        self.scan_ports_button.grid(row=0, column=2, rowspan=3, padx=10, pady=5)

        self.scan_host_button = tk.Button(self.master, text="Scan Host Info", command=self.scan_host_info)
        self.scan_host_button.grid(row=0, column=3, rowspan=3, padx=10, pady=5)

    def create_result_widgets(self):
        self.result_frame = tk.Frame(self.master)
        self.result_frame.grid(row=3, column=0, columnspan=4, padx=10, pady=5)

        self.create_result_labels()
        self.create_result_texts()

    def create_result_labels(self):
        self.result_label_ports = tk.Label(self.result_frame, text="Open Ports:")
        self.result_label_ports.grid(row=0, column=0, sticky="w", padx=10, pady=5)

        self.result_label_host = tk.Label(self.result_frame, text="Host Information:")
        self.result_label_host.grid(row=0, column=1, sticky="w", padx=10, pady=5)

    def create_result_texts(self):
        self.result_text_ports = tk.Text(self.result_frame, height=10, width=50)
        self.result_text_ports.grid(row=1, column=0, padx=10, pady=5)

        self.result_text_host = tk.Text(self.result_frame, height=10, width=50)
        self.result_text_host.grid(row=1, column=1, padx=10, pady=5)

    def scan_port(self, host, port):
        packet = IP(dst=host)/TCP(dport=port, flags="S")
        response = sr1(packet, timeout=1, verbose=0)
        result = f"Port {port} is {'open' if response and response.haslayer(TCP) and response.getlayer(TCP).flags == 0x12 else 'closed'}\n"
        self.result_text_ports.insert(tk.END, result)

    def scan_host_info(self):
        host = self.entry_target.get()
        self.result_text_host.delete("1.0", tk.END)
        try:
            ip = socket.gethostbyname(host)
            host_info = requests.get(f'http://ip-api.com/json/{ip}?fields=country,regionName,city,lat,lon,org').json()
            result_text = "\nHost Information:\n"
            for key, value in host_info.items():
                result_text += f"{key}: {value}\n"
            self.result_text_host.insert(tk.END, result_text)
        except socket.gaierror:
            self.result_text_host.insert(tk.END, "Hostname could not be resolved\n")
        except Exception as e:
            self.result_text_host.insert(tk.END, f"Error getting host information: {e}\n")

    def scan_ports(self):
        host = self.entry_target.get()
        start_port_str = self.entry_start_port.get()
        end_port_str = self.entry_end_port.get()
        
        if not start_port_str or not end_port_str:
            self.result_text_ports.insert(tk.END, "Please enter valid start and end ports\n")
            return
        
        try:
            start_port = int(start_port_str)
            end_port = int(end_port_str)
            
            # Ограничение интервала портов до 1000
            if end_port - start_port > 1000:
                self.result_text_ports.insert(tk.END, "Port range cannot exceed 1000\n")
                return
            
            self.result_text_ports.delete("1.0", tk.END)
            ip = socket.gethostbyname(host)
            for port in range(start_port, end_port + 1):
                threading.Thread(target=self.scan_port, args=(ip, port)).start()
        except socket.gaierror:
            self.result_text_ports.insert(tk.END, "Hostname could not be resolved\n")
        except ValueError:
            self.result_text_ports.insert(tk.END, "Please enter valid start and end ports\n")
        except Exception as e:
            self.result_text_ports.insert(tk.END, f"Error scanning ports: {e}\n")

def main():
    root = tk.Tk()
    gui = PortScannerGUI(root)
    root.mainloop()

if __name__ == "__main__":
    main()

