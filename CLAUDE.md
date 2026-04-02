# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**AmazonOrders** is a Qt6/C++ desktop application that automates Amazon inventory order creation. It processes Amazon inventory recommendation reports and generates purchase order XLSX files with embedded product images, supporting multiple Amazon marketplaces (DE, CA, US, UK, JP).

## Build Commands

```bash
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=/path/to/Qt6 -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

Qt Creator (`*.user` files) is the expected IDE. The project uses Qt's AUTO MOC/UIC/RCC — do not manually run these tools.

## Architecture

### Data Flow

1. **Input:** User adds XLSX order template files + imports an Amazon inventory recommendation CSV (or pastes from clipboard)
2. **Filter (optional):** `DialogFilterOut` reduces the recommendation table by SKU/name patterns
3. **Output:** `OrderCreator` merges templates with recommendations and writes a new XLSX with embedded product images

### Key Classes

| Class | Role |
|-------|------|
| `MainWindow` | UI orchestration — coordinates all components, handles country selection |
| `TableInventoryRecommendation` | Singleton QAbstractTableModel holding parsed inventory data (9 columns: SKU, Title, Units Sold 30d, Unit Supply, Day Supply, Recommended Qty, Price, FNSKU, ASIN) |
| `OrderCreator` | Parses input XLSX templates, matches SKU→recommendations, embeds images, writes output XLSX |
| `ListOrderModel` | QAbstractListModel managing the list of input XLSX file paths (persisted per country) |
| `DialogFilterOut` | Whitelist/blacklist filtering by SKU or product name patterns, with saved templates |

### External Dependencies

- **Qt6::Widgets** — all UI
- **QXlsx** — reading and writing `.xlsx` files
- **`../common/`** (sibling directory) — shared utilities: `WorkingDirectoryManager`, `CsvReader`, `types.h`

### Persistence

All state is stored via `QSettings` (org: `Icinger Power`, app: `Amazon Order Maker`). Settings keys are suffixed by country code for multi-locale isolation (e.g., `ListOrderModelDE`, `TableInventoryRecommendationCA`).

### SKU/Image Matching

`OrderCreator` scans an image directory for files named after SKUs. It handles CJ-brand variant logic: if `CJ-XXX-02` has no image, it falls back to `CJ-XXX`. Size variants ending in `NARROW`/`WIDE` follow the same parent-fallback pattern.

### CSV Import Heuristics

`TableInventoryRecommendation::importCsvRecommendation()` applies quantity corrections:
- Days-to-ship 30–120 days → ×0.8 multiplier
- 0 units in stock → ×1.3 multiplier

Items with 0 units or <45 days supply are flagged for reorder.
