#!/bin/bash

echo "╔════════════════════════════════════════════╗"
echo "║   SwiftEx Courier System - Quick Start    ║"
echo "╚════════════════════════════════════════════╝"
echo ""

if ! command -v node &> /dev/null; then
    echo "❌ Node.js is not installed!"
    echo "Please install Node.js from https://nodejs.org/"
    exit 1
fi

echo "✓ Node.js found: $(node --version)"

if ! command -v npm &> /dev/null; then
    echo "❌ npm is not installed!"
    exit 1
fi

echo "✓ npm found: $(npm --version)"
echo ""

if [ ! -d "node_modules" ]; then
    echo "📦 Installing dependencies..."
    npm install
    echo ""
fi

if [ ! -d "data" ]; then
    echo "❌ Data directory not found!"
    exit 1
fi

echo "✓ Data directory found"
echo ""

echo "🚀 Starting SwiftEx Courier System..."
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "   Access the application at:"
echo "   http://localhost:3000"
echo ""
echo "   Default Login:"
echo "   Username: admin"
echo "   Password: admin123"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "Press Ctrl+C to stop the server"
echo ""

npm start