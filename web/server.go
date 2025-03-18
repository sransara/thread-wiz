package main

import (
    "bufio"
    "context"
    "fmt"
    "net/http"
    "os"
    "path/filepath"
    "sync"

    "github.com/coder/websocket"
)

func main() {
    html, err := os.ReadFile(filepath.Join("web", "index.html"))
    if err != nil {
        fmt.Println("Error reading HTML file:", err)
        return
    }

    http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
        w.Header().Set("Content-Type", "text/html")
        w.Write(html)
    })

    http.HandleFunc("/ws", handleWebSocket)
    fmt.Println("WebSocket server started on http://localhost:8080")
    err = http.ListenAndServe(":8080", nil)
    if err != nil {
        fmt.Println("Error starting server:", err)
    }
}

func handleWebSocket(w http.ResponseWriter, r *http.Request) {
    conn, err := websocket.Accept(w, r, nil)
    if err != nil {
        fmt.Println("Error accepting WebSocket connection:", err)
        return
    }
    defer conn.Close(websocket.StatusInternalError, "Internal server error")

    fmt.Println("WebSocket connection established")

    ctx, cancel := context.WithCancel(r.Context())
    defer cancel()

    var wg sync.WaitGroup
    wg.Add(1)

    go func() {
        defer wg.Done()
        readStdinAndSend(ctx, conn)
    }()

    for {
        _, msg, err := conn.Read(ctx)
        if err != nil {
            fmt.Println("Error reading message:", err)
            break
        }
        fmt.Printf("Received message: %s\n", msg)

        err = conn.Write(ctx, websocket.MessageText, []byte(fmt.Sprintf("Message text was: %s", msg)))
        if err != nil {
            fmt.Println("Error writing message:", err)
            break
        }
    }

    cancel()
    wg.Wait()
    conn.Close(websocket.StatusNormalClosure, "Normal closure")
}

func readStdinAndSend(ctx context.Context, conn *websocket.Conn) {
    scanner := bufio.NewScanner(os.Stdin)
    for scanner.Scan() {
        select {
        case <-ctx.Done():
            return
        default:
            line := scanner.Text()
            if line != "" {
                err := conn.Write(ctx, websocket.MessageText, []byte(line))
                if err != nil {
                    fmt.Println("Error writing stdin message:", err)
                    return
                }
            }
        }
    }
    if err := scanner.Err(); err != nil {
        fmt.Println("Error reading stdin:", err)
    }
}
