#!/bin/bash

set -e  # Exit immediately on error

# Parse arguments
run=false
while [[ $# -gt 0 ]]; do
  case "$1" in
    /R|-r|--run)
      run=true
      shift
      ;;
    *)
      shift
      ;;
  esac
done

# Build frontend
echo "Building frontend..."
cd frontend
npm run build || { echo "Frontend build failed"; exit 1; }
cd ..

# Copy frontend build to backend/static
echo "Copying frontend build to backend/static..."
rm -rf backend/static
mkdir -p backend/static
cp -r frontend/build/* backend/static/

# Build backend
echo "Building backend..."
cd backend
go build -o ../bin/argus main.go || { echo "Backend build failed"; exit 1; }
cd ..

echo "Build complete"

# Optionally run the binary
if [ "$run" = true ]; then
  echo "Running ./bin/argus"
  ./bin/argus
else
  echo "Run with: ./bin/argus"
fi
