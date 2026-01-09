#!/usr/bin/env python3
"""
Dashboard: Independent Python application to display universe statistics.
Connects to the server via ZMQ and periodically requests state snapshots.
Displays:
  - Planets with recycled trash-ship count
  - Trash-ships with current cargo
  - Total roaming trash and capacity percentage
"""

import zmq
import time
import sys
import os
import re

# Add shared protobuf path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'shared'))

try:
    import ship_movement_pb2 as ship_movement
except ImportError:
    print("Error: Could not import ship_movement_pb2. Make sure protobuf is generated.")
    print("Run: protoc --python_out=. ship_movement.proto from PartB/shared/")
    sys.exit(1)


def load_config(config_path):
    """
    Load configuration from libconfig-style config file.
    
    Args:
        config_path: Path to config file
    
    Returns:
        dict with config values or None on error
    """
    config = {}
    try:
        with open(config_path, 'r') as f:
            for line in f:
                line = line.strip()
                # Skip comments and empty lines
                if not line or line.startswith('#'):
                    continue
                
                # Parse: key = "value" or key = number
                match = re.match(r'(\w+)\s*=\s*["\']?([^"\']+)["\']?', line)
                if match:
                    key, value = match.groups()
                    # Try to convert to int if it looks like a number
                    try:
                        config[key] = int(value)
                    except ValueError:
                        config[key] = value.strip('"\'')
        
        return config if config else None
    except FileNotFoundError:
        print(f"[Dashboard] Config file not found: {config_path}")
        return None
    except Exception as e:
        print(f"[Dashboard] Error loading config: {e}")
        return None


class Dashboard:
    """Dashboard client that displays universe state."""
    
    def __init__(self, server_host='127.0.0.1', server_port=5555):
        """
        Initialize dashboard.
        
        Args:
            server_host: Server address (default: localhost)
            server_port: Server port (default: 5555)
        """
        self.server_host = server_host
        self.server_port = server_port
        self.context = None
        self.socket = None
        self.connected = False
        self.consecutive_failures = 0
        self.max_failures = 2  # Exit after 2 consecutive failures
    
    def connect(self):
        """Connect to the server via ZMQ."""
        try:
            self.context = zmq.Context()
            self.socket = self.context.socket(zmq.REQ)
            
            # Set linger to 0 to avoid blocking on close
            self.socket.setsockopt(zmq.LINGER, 0)
            # Set timeout to avoid hanging
            self.socket.setsockopt(zmq.RCVTIMEO, 5000)  # 5 second timeout
            
            connection_string = f"tcp://{self.server_host}:{self.server_port}"
            self.socket.connect(connection_string)
            
            self.connected = True
            print(f"[Dashboard] Connected to server at {connection_string}")
            return True
        except Exception as e:
            print(f"[Dashboard] Connection failed: {e}")
            self.connected = False
            return False
    
    def request_state(self):
        """
        Request state snapshot from server.
        
        Returns:
            StateSnapshot or None if request fails
        """
        if not self.connected:
            return None
        
        try:
            # Build STATE_REQUEST envelope
            env = ship_movement.Envelope()
            env.type = ship_movement.Envelope.STATE_REQUEST
            
            # Send request
            self.socket.send(env.SerializeToString())
            
            # Receive response
            response_data = self.socket.recv()
            
            # Parse response as ServerResponse (not Envelope)
            response = ship_movement.ServerResponse()
            response.ParseFromString(response_data)
            
            # Check if state is present
            if response.HasField('state'):
                self.consecutive_failures = 0  # Reset on success
                return response.state
            else:
                print(f"[Dashboard] Response missing state field")
                self.consecutive_failures += 1
                return None
        except zmq.error.Again:
            print("[Dashboard] Request timeout - server did not respond")
            self.consecutive_failures += 1
            # Recreate socket after timeout to reset state
            self._recreate_socket()
            return None
        except Exception as e:
            print(f"[Dashboard] Error requesting state: {e}")
            self.consecutive_failures += 1
            # Recreate socket after error to reset state
            self._recreate_socket()
            return None
    
    def _recreate_socket(self):
        """Recreate socket to reset REQ state machine."""
        try:
            if self.socket:
                # Set linger to 0 before closing
                self.socket.setsockopt(zmq.LINGER, 0)
                self.socket.close()
            self.socket = self.context.socket(zmq.REQ)
            self.socket.setsockopt(zmq.LINGER, 0)
            self.socket.setsockopt(zmq.RCVTIMEO, 5000)
            connection_string = f"tcp://{self.server_host}:{self.server_port}"
            self.socket.connect(connection_string)
        except Exception as e:
            print(f"[Dashboard] Failed to recreate socket: {e}")
            self.connected = False
    
    def display_state(self, snapshot):
        """
        Display state snapshot in formatted output.
        
        Args:
            snapshot: StateSnapshot protobuf object
        """
        if snapshot is None:
            print("[Dashboard] No snapshot to display")
            return
        
        print("\n" + "="*50)
        print(f"UNIVERSE STATE")
        print("="*50)
        
        # Display planets with recycled trash counts
        print("\nPLANETS (Recycled trash)")
        if snapshot.planets:
            for i, planet in enumerate(snapshot.planets):
                recycled = planet.recycled_count
                recycling_status = " <- RECYCLING" if planet.is_recycling else ""
                print(f"{chr(65 + i)} : {recycled}{recycling_status}")
        else:
            print("  (No planets)")
        
        # Display trash-ships with cargo
        print("\nTRASH-SHIPS (Trash Cargo)")
        connected_ships = 0
        total_cargo = 0
        if snapshot.ships:
            for i, ship in enumerate(snapshot.ships):
                if ship.connected:
                    connected_ships += 1
                    ship_id = chr(65 + i) if i < 26 else chr(97 + i - 26)
                    total_cargo += ship.load
                    print(f"  {ship_id} - {ship.load}")
        
        if connected_ships == 0:
            print("  (No ships connected)")
        
        # Display total roaming trash and capacity
        total_trash = len(snapshot.trash)
        # max_trash not in snapshot, estimate or use a default
        max_trash = 300  # from server config
        trash_percentage = (total_trash / max_trash * 100) if max_trash > 0 else 0
        
        print("\nUNIVERSE")
        print(f"  ROAMING TRASH: {total_trash}")
        print(f"  TRASH CAPACITY: {trash_percentage:.1f}%")
        
        print("="*50 + "\n")
    
    def run(self, update_interval=1.0):
        """
        Run the dashboard loop.
        
        Args:
            update_interval: Seconds between state requests (default: 1.0)
        """
        if not self.connect():
            sys.exit(1)
        
        print("[Dashboard] Starting main loop (press Ctrl+C to exit)")
        
        try:
            while True:
                # Check if too many failures
                if self.consecutive_failures >= self.max_failures:
                    print(f"\n[Dashboard] Server disconnected ({self.max_failures} consecutive failures)")
                    print("[Dashboard] Exiting...")
                    break

                # Request and display state
                snapshot = self.request_state()
                if snapshot:
                    self.display_state(snapshot)

                # Wait before next request
                time.sleep(update_interval)
        except KeyboardInterrupt:
            print("\n[Dashboard] Shutting down...")
        except Exception as e:
            print(f"[Dashboard] Error in main loop: {e}")
        finally:
            self.cleanup()
            sys.exit(0)
    
    def cleanup(self):
        """Clean up ZMQ resources."""
        if self.socket:
            # Set linger to 0 to avoid blocking on close
            self.socket.setsockopt(zmq.LINGER, 0)
            self.socket.close()
        if self.context:
            self.context.term()
        self.connected = False
        print("[Dashboard] Cleanup complete")


def main():
    """Main entry point."""
    # Try to load config from dashboard_init.conf
    config_path = os.path.join(os.path.dirname(__file__), 'dashboard_init.conf')
    
    server_host = '127.0.0.1'
    server_port = 5555
    
    config = load_config(config_path)
    if config:
        # Map config keys to variables
        if 'server_address_str' in config:
            server_host = config['server_address_str']
        if 'server_port_int' in config:
            server_port = config['server_port_int']
        print(f"[Dashboard] Loaded config from {config_path}")
    
    # Allow command-line args to override config
    if len(sys.argv) > 1:
        server_host = sys.argv[1]
    if len(sys.argv) > 2:
        server_port = int(sys.argv[2])
    
    print(f"[Dashboard] Connecting to {server_host}:{server_port}")
    
    dashboard = Dashboard(server_host, server_port)
    dashboard.run(update_interval=2.0)  # Update every 2 seconds


if __name__ == '__main__':
    main()
