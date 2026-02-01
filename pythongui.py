import tkinter as tk
from tkinter import ttk, messagebox
import serial
import serial.tools.list_ports
import mido
import threading

class SimpleFloppyBridge:
    def __init__(self, root):
        self.root = root
        self.root.title("Floppy MIDI Bridge")
        self.root.geometry("420x260")

        self.serial_conn = None
        self.midi_in = None
        self.running = False

        tk.Label(root, text="COM Port:").pack(pady=5)
        self.port_combo = ttk.Combobox(root, values=self.get_ports(), width=35)
        self.port_combo.pack(pady=5)
        tk.Button(root, text="Refresh Ports", command=self.refresh_ports).pack(pady=2)

        tk.Label(root, text="MIDI Input:").pack(pady=5)
        self.midi_combo = ttk.Combobox(root, values=mido.get_input_names(), width=35)
        self.midi_combo.pack(pady=5)
        tk.Button(root, text="Refresh MIDI", command=self.refresh_midi).pack(pady=2)

        self.start_btn = tk.Button(
            root, text="Start Bridge", bg="green", fg="white",
            width=20, command=self.toggle_bridge
        )
        self.start_btn.pack(pady=10)

        self.status = tk.Label(root, text="Idle")
        self.status.pack(pady=5)

    def get_ports(self):
        return [p.device for p in serial.tools.list_ports.comports()]

    def refresh_ports(self):
        self.port_combo["values"] = self.get_ports()

    def refresh_midi(self):
        self.midi_combo["values"] = mido.get_input_names()

    def toggle_bridge(self):
        if not self.running:
            self.start_bridge()
        else:
            self.stop_bridge()

    def start_bridge(self):
        port = self.port_combo.get()
        midi_name = self.midi_combo.get()

        if not port or not midi_name:
            messagebox.showwarning("Error", "Select COM port and MIDI input.")
            return

        try:
            self.serial_conn = serial.Serial(port, 115200, timeout=0.01)
        except Exception as e:
            messagebox.showerror("Serial Error", str(e))
            return

        try:
            self.midi_in = mido.open_input(midi_name)
        except Exception as e:
            messagebox.showerror("MIDI Error", str(e))
            self.serial_conn.close()
            self.serial_conn = None
            return

        self.running = True
        self.start_btn.config(text="Stop Bridge", bg="red")
        self.status.config(text=f"Running: {midi_name} → {port}")

        threading.Thread(target=self.midi_worker, daemon=True).start()

    def stop_bridge(self):
        self.running = False

        if self.midi_in:
            self.midi_in.close()
            self.midi_in = None

        if self.serial_conn:
            self.serial_conn.close()
            self.serial_conn = None

        self.start_btn.config(text="Start Bridge", bg="green")
        self.status.config(text="Stopped")

    def midi_worker(self):
        try:
            while self.running and self.midi_in and self.serial_conn:
                for msg in self.midi_in.iter_pending():
                    if msg.type in ("note_on", "note_off"):
                        try:
                            # sends raw MIDI bytes
                            self.serial_conn.write(msg.bytes())
                        except Exception:
                            self.root.after(0, self.stop_bridge)
                            return
        except Exception:
            self.root.after(0, self.stop_bridge)

if __name__ == "__main__":
    root = tk.Tk()
    app = SimpleFloppyBridge(root)
    root.mainloop()