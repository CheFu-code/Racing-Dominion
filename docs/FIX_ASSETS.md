# Asset Download Issue - FIXED

## Problem

The previous workflows downloaded HTML files instead of actual .glb models because:

1. **Wrong URLs**: Used GitHub web URLs that return HTML pages
2. **No validation**: Didn't check if downloaded files were valid glTF
3. **Silent failures**: Workflows succeeded even when downloads failed

## Solution

New workflow: `.github/workflows/download-real-assets.yml`

### Key Fixes:

1. **Proper URLs**: Use `raw.githubusercontent.com` for binary downloads
2. **File validation**: Python script checks glTF magic number (`glTF` header)
3. **Auto-cleanup**: Deletes corrupt/HTML files automatically
4. **Verbose logging**: Shows success/failure for each download

### Validation Process:

```python
# Checks each .glb file for:
- glTF magic number (0x46546C67)
- File size > 100 bytes
- Not HTML content
```

Invalid files are automatically deleted before commit.

## Run Fixed Workflow

1. Go to [Actions tab](https://github.com/CheFu-code/Racing-Dominion/actions)
2. Select **"Download Real 3D Assets (Fixed)"**
3. Click **"Run workflow"**
4. Wait 2-3 minutes
5. Check commit for validated assets

## Current Models

The workflow downloads **placeholder models** from Khronos glTF samples:

- ✅ Verified binary glTF format
- ✅ Valid 3D geometry
- ⚠️ Not actual racing cars (placeholders)

## Getting Real Racing Cars

### Manual Method (Recommended):

1. **Download from Sketchfab**:
   - Visit: https://sketchfab.com/search?q=racing+car&type=models
   - Filter: Downloadable, Free
   - Format: glTF Binary (.glb)
   - License: CC-BY or CC0

2. **Add to repository**:
   ```bash
   # Clone repo
   git clone https://github.com/CheFu-code/Racing-Dominion.git
   cd Racing-Dominion
   
   # Add model
   cp ~/Downloads/racing_car.glb app/src/main/assets/models/cars/
   
   # Commit
   git add app/src/main/assets/models/cars/racing_car.glb
   git commit -m "Add real racing car model"
   git push
   ```

3. **Update config**:
   Edit `app/src/main/assets/asset_config.json`:
   ```json
   {
     "vehicles": [
       {
         "id": "racing_car",
         "name": "Street Racer",
         "model": "models/cars/racing_car.glb",
         "stats": {
           "top_speed": 200,
           "acceleration": 8.5
         }
       }
     ]
   }
   ```

### Automated Method:

For public domain models, you can add URLs to the workflow:

```yaml
- name: Download custom car
  run: |
    curl -L -f -o custom_car.glb "YOUR_DIRECT_GLB_URL_HERE"
```

## Free Racing Car Sources

### High Quality:
1. **Sketchfab** (requires account): https://sketchfab.com
   - Search: "racing car"
   - Filter: CC-BY, Downloadable
   - Format: glTF Binary

2. **TurboSquid Free**: https://www.turbosquid.com/Search/3D-Models/free/car
   - Free section has some cars
   - May need conversion to .glb

3. **Poly Haven**: https://polyhaven.com/models
   - All CC0 (public domain)
   - High quality
   - Limited vehicle selection

### Quick Placeholders:
4. **Kenney Assets**: https://kenney.nl/assets
   - Free low-poly vehicles
   - Game-ready
   - CC0 license

## Validation Tool

Check if a .glb file is valid:

```bash
# Check file header
xxd -l 4 your_model.glb
# Should show: 676c 5446 ("glTF" in hex)

# Check file size
ls -lh your_model.glb
# Should be > 1KB for simple models
```

## Testing Models

Before adding to repository:

1. **View online**: Upload to https://gltf-viewer.donmccurdy.com/
2. **Validate**: Use https://github.khronos.org/glTF-Validator/
3. **Check license**: Ensure commercial use allowed

## Troubleshooting

### If download still fails:

1. Check workflow logs in Actions tab
2. Look for "✗" markers in validation output
3. File size should be > 10KB for real models
4. Use manual method as fallback

### Common Issues:

- **0 byte files**: Download failed, URL incorrect
- **~297KB files**: HTML error page downloaded
- **No glTF header**: Corrupt or wrong format

## Need Help?

Open an issue with:
- Model source URL you're trying to use
- Error messages from workflow
- Screenshot of validation output