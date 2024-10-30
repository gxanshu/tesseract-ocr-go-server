package main

// #cgo LDFLAGS: -ltesseract -llept
// #include <stdlib.h>  // Needed for free()
// #include "tesseract.h"
import "C"
import (
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"unsafe"
)

func main() {
	http.HandleFunc("/extract", func(w http.ResponseWriter, r *http.Request) {
		if r.Method != http.MethodPost {
			http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
			return
		}

		// Parse multipart form with 10MB max memory
		err := r.ParseMultipartForm(10 << 20)
		if err != nil {
			http.Error(w, "Error parsing form", http.StatusBadRequest)
			return
		}

		// Get file from form
		file, _, err := r.FormFile("image")
		if err != nil {
			http.Error(w, "Error retrieving image file", http.StatusBadRequest)
			return
		}
		defer file.Close()

		// Read file bytes
		imageBytes, err := io.ReadAll(file)
		if err != nil {
			http.Error(w, "Error reading image file", http.StatusInternalServerError)
			return
		}

		size := C.int(len(imageBytes))

		// Call the C function
		result := C.extract_text_from_image((*C.uchar)(&imageBytes[0]), size)

		// Convert the C string result to Go string
		text := C.GoString(result)

		// Free the memory allocated by strdup in C
		C.free(unsafe.Pointer(result))

		// Return extracted text as JSON response
		w.Header().Set("Content-Type", "application/json")
		json.NewEncoder(w).Encode(map[string]string{
			"text": text,
		})
	})

	fmt.Println("Server starting on port 8080...")
	http.ListenAndServe(":8080", nil)
}
