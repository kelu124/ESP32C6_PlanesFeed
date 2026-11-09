# Describe the design in the README and add an MIT license

Date: 2026-11-09

## What changed
- **README:** replaced the title-only file with what the tool does, its screens, the design and known limits, the code layout, setup, the make targets and the USB CLI.
- **License:** added `LICENSE` with the MIT text, and a README section linking to it.

## Why
The repository had no description of the tool, and no license, so others had no stated right to reuse the code. The README states that the MIT license covers the code only, because adsb.fi limits its data to personal, non-commercial use and adsbdb restricts copying its route data.

## Testing
`make test` passed. No firmware code changed, so the firmware was not rebuilt or flashed. Mermaid rendering of the README diagram was not checked on GitHub.
