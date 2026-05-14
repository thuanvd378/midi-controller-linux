import threading
import tkinter as tk

MIDI_DEV = "/dev/custom_midi"

class VirtualKeyboard:
    def __init__(self, root):
        self.root = root
        self.root.title("Virtual Piano - MIDI Controller")
        self.root.configure(bg="#2b2b2b")
        
        self.keys = {}
        
        # Display title
        title = tk.Label(self.root, text="🎹 Virtual MIDI Keyboard", font=("Helvetica", 20, "bold"), fg="white", bg="#2b2b2b")
        title.pack(pady=10)
        
        # Ensure 61 keys (5 octaves starting from C2)
        self.start_note = 36 # C2
        self.num_keys = 61 # 5 octaves
        
        white_key_width = 40
        num_white_keys = len([i for i in range(self.num_keys) if (self.start_note + i) % 12 in [0, 2, 4, 5, 7, 9, 11]])
        canvas_width = num_white_keys * white_key_width
        
        self.canvas = tk.Canvas(root, width=canvas_width, height=250, bg="#1e1e1e", highlightthickness=0)
        self.canvas.pack(padx=30, pady=20)
        
        self.draw_keyboard(white_key_width)
        
        self.status_var = tk.StringVar()
        self.status_var.set(f"Waiting for MIDI input from {MIDI_DEV}...")
        status = tk.Label(self.root, textvariable=self.status_var, font=("Helvetica", 12), fg="#a0a0a0", bg="#2b2b2b")
        status.pack(pady=10)
        
        self.running = True
        self.midi_thread = threading.Thread(target=self.read_midi)
        self.midi_thread.daemon = True
        self.midi_thread.start()
        
        self.root.protocol("WM_DELETE_WINDOW", self.on_close)
        
    def draw_keyboard(self, white_key_width):
        white_key_height = 200
        black_key_width = 24
        black_key_height = 130
        
        x = 0
        white_notes = [0, 2, 4, 5, 7, 9, 11]
        
        # Draw white keys
        for i in range(self.num_keys):
            note = self.start_note + i
            if note % 12 in white_notes:
                rect = self.canvas.create_rectangle(
                    x, 0, x + white_key_width, white_key_height, 
                    fill="#f0f0f0", outline="#333", width=2
                )
                self.keys[note] = {"id": rect, "type": "white", "pressed": False}
                x += white_key_width
                
        # Draw black keys
        x = 0
        for i in range(self.num_keys):
            note = self.start_note + i
            if note % 12 in white_notes:
                if i < self.num_keys - 1 and (note + 1) % 12 not in white_notes:
                    rect = self.canvas.create_rectangle(
                        x + white_key_width - black_key_width//2, 0, 
                        x + white_key_width + black_key_width//2, black_key_height, 
                        fill="#222222", outline="#111", width=1
                    )
                    self.keys[note + 1] = {"id": rect, "type": "black", "pressed": False}
                x += white_key_width
                
    def on_close(self):
        self.running = False
        self.root.destroy()
        
    def read_midi(self):
        try:
            with open(MIDI_DEV, "rb") as f:
                self.status_var.set(f"Connected to {MIDI_DEV}! Play your keyboard.")
                while self.running:
                    byte1 = f.read(1)
                    if not byte1: continue
                    b1 = byte1[0]
                    
                    if (b1 & 0xF0) == 0x90 or (b1 & 0xF0) == 0x80:
                        b2 = f.read(1)[0]
                        b3 = f.read(1)[0]
                        
                        cmd = b1 & 0xF0
                        note = b2
                        velocity = b3
                        
                        if cmd == 0x90 and velocity > 0:
                            self.root.after(0, self.key_on, note)
                        elif cmd == 0x80 or (cmd == 0x90 and velocity == 0):
                            self.root.after(0, self.key_off, note)
        except Exception as e:
            self.status_var.set(f"Error reading {MIDI_DEV}: {e} (Run driver first?)")
            
    def key_on(self, note):
        if note in self.keys:
            color = "#ff6b6b" if self.keys[note]["type"] == "white" else "#c92a2a"
            self.canvas.itemconfig(self.keys[note]["id"], fill=color)
            
    def key_off(self, note):
        if note in self.keys:
            color = "#f0f0f0" if self.keys[note]["type"] == "white" else "#222222"
            self.canvas.itemconfig(self.keys[note]["id"], fill=color)

if __name__ == "__main__":
    root = tk.Tk()
    app = VirtualKeyboard(root)
    root.mainloop()
