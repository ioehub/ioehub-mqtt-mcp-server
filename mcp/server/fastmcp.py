"""
Simple FastMCP implementation for MQTT MCP server.
"""
import json
import sys

class FastMCP:
    """
    FastMCP class for creating MCP tools.
    """
    def __init__(self):
        self.tools = {}
    
    def tool(self, name=None):
        """
        Decorator for registering tools.
        
        :param name: Optional name for the tool. If not provided, the function name is used.
        :return: Decorator function
        """
        def decorator(func):
            tool_name = name or func.__name__
            self.tools[tool_name] = func
            return func
        return decorator
    
    def run(self, transport="stdio"):
        """
        Run the MCP server with the specified transport.
        
        :param transport: Transport type ("stdio" or "tcp")
        """
        if transport == "stdio":
            self._run_stdio()
        else:
            raise ValueError(f"Transport {transport} not supported")
    
    def _run_stdio(self):
        """
        Run the server with stdio transport.
        """
        print("MCP server running with stdio transport")
        print("Waiting for commands...")
        
        while True:
            try:
                line = sys.stdin.readline().strip()
                if not line:
                    continue
                
                try:
                    request = json.loads(line)
                    method = request.get("method")
                    
                    if method and method in self.tools:
                        params = request.get("params", {})
                        result = self.tools[method](**params)
                        
                        response = {
                            "jsonrpc": "2.0",
                            "id": request.get("id"),
                            "result": result
                        }
                    else:
                        response = {
                            "jsonrpc": "2.0",
                            "id": request.get("id"),
                            "error": {
                                "code": -32601,
                                "message": f"Method {method} not found"
                            }
                        }
                    
                    print(json.dumps(response))
                    sys.stdout.flush()
                    
                except json.JSONDecodeError:
                    print(json.dumps({
                        "jsonrpc": "2.0",
                        "id": None,
                        "error": {
                            "code": -32700,
                            "message": "Parse error"
                        }
                    }))
                    sys.stdout.flush()
                    
            except KeyboardInterrupt:
                print("MCP server stopped")
                break 