import serial
import time
from pycaw.pycaw import AudioUtilities, IAudioEndpointVolume
from ctypes import cast, POINTER
from comtypes import CLSCTX_ALL

# --- CONFIG ---
COM_PORT = 'COM5'
BAUD_RATE = 115200
last_sync_time = 0
SYNC_INTERVAL = 2  # Seconds between syncs
master_lvl = 0.5
discord_lvl = 0.5
game_level = 0.5
firefox_lvl = 0.5

# Add your games here
GAME_LIST = [
    "VALORANT-Win64-Shipping.exe", 
    "cs2.exe", 
    "FortniteClient-Win64-Shipping.exe", 
    "Overwatch.exe",
    "EldenRing.exe"
]

def get_master_interface():
    devices = AudioUtilities.GetDeviceEnumerator()
    endpoint = devices.GetDefaultAudioEndpoint(0, 0)
    return endpoint.Activate(IAudioEndpointVolume._iid_, CLSCTX_ALL, None)

def set_app_volume(app_name, level):
    try:
        sessions = AudioUtilities.GetAllSessions()
        found_any = False
        
        for session in sessions:
            if session.Process:
                # We check the name carefully
                current_name = session.Process.name()
                if current_name.lower() == app_name.lower():
                    # Update THIS specific session
                    session.SimpleAudioVolume.SetMasterVolume(level, None)
                    found_any = True
                    # CRITICAL: We do NOT 'return True' here anymore. 
                    # We let the loop continue to find the SECOND Discord entry.
        
        return found_any
    except Exception as e:
        print(f"Error setting volume for {app_name}: {e}")
        return False

# --- ADDED THIS MISSING FUNCTION ---
def set_group_volume(exe_list, level):
    sessions = AudioUtilities.GetAllSessions()
    for session in sessions:
        if session.Process and session.Process.name() in exe_list:
            session.SimpleAudioVolume.SetMasterVolume(level, None)

# Initialize
try:
    py_serial = serial.Serial(COM_PORT, BAUD_RATE, timeout=0.1)
    master_interface = get_master_interface()
    master_vol = cast(master_interface, POINTER(IAudioEndpointVolume))
    print("System Active. Monitoring M5Stack...")
except Exception as e:
    print(f"Startup Error: {e}")
    time.sleep(5) # Keeps window open so you can read the error
    exit()

while True:
    try:
        if py_serial.in_waiting > 100:
            py_serial.reset_input_buffer()
            continue 

        if py_serial.in_waiting > 0:
            line = py_serial.readline().decode('utf-8', errors='ignore').strip()
            
            if "|" in line:
                vals = line.split('|')
                if len(vals) >= 4:
                    # Slider 0 -> Master
                    master_lvl = int(vals[0]) / 1023.0
                    master_vol.SetMasterVolumeLevelScalar(master_lvl, None)

                    # Slider 1 -> Discord
                    discord_lvl = int(vals[1]) / 1023.0
                    set_app_volume("Discord.exe", discord_lvl)

                    # Slider 2 -> Games
                    game_level = int(vals[2]) / 1023.0
                    set_group_volume(GAME_LIST, game_level)

                    # Slider 3 -> Firefox
                    firefox_lvl = int(vals[3]) / 1023.0
                    set_app_volume("Firefox.exe", firefox_lvl)
                    
                    # Fixed the variable name from games_lvl to game_level
                    print(f"M: {int(master_lvl*100)}% | D: {int(discord_lvl*100)}% | G: {int(game_level*100)}% | F: {int(firefox_lvl*100)}%")
     
        current_time = time.time()
        if current_time - last_sync_time > SYNC_INTERVAL:
            try:
                m_vol = int(master_vol.GetMasterVolumeLevelScalar() * 1023)
                # You can expand this to send App volumes too
                sync_msg = f"SYNC|{master_lvl*10.23}|{discord_lvl*10.23}|{game_lvl*10.23}|{firefox_lvl*10.23}\n" 
                py_serial.write(sync_msg.encode())
                last_sync_time = current_time
            except:
                pass
            
    except Exception as e:
        if "Activate" in str(e):
             master_interface = get_master_interface()
             master_vol = cast(master_interface, POINTER(IAudioEndpointVolume))
        print(f"Error: {e}")
    time.sleep(0.01)