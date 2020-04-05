# rsgeotools
Toolset related to OpenStreetMap 3D renderer and/or vector tiles

This repository is currently under development. Please come back later. 

## Dependencies

* libgeos_c, LGPL-licensed, for rsgeotools-csv2rvtdata
* gdal, MIT/X style license, for processing heightmaps with rsgeotools-conv and for processing geodata
* libtiff-dev and geotiff-bin, for processing TIFF images
* osmctools (osmconvert and osmfilter) for initial processing OSM planet

# Usage

## 1. Prepare OSM Planet

1. Download OSM Planet from https://planet.openstreetmap.org/ (PBF). 
2. Run `RVT_O5M_DIR=<o5m dir> planet-init.sh <planet-******.osm.pbf>`

Planet will be subdivided to zoom 7 tiles and saved to `$RVT_O5M_DIR` in o5m format.

## 2. Prepare ocean (waterpoly)

In OpenStreetMap, large water areas are not stored as polygons, instead, `natural=coastline` tag is used (read more: https://wiki.openstreetmap.org/wiki/Coastline). Ready-to-use polygons are provided by osmdata.openstreetmap.de.

1. Download `water-polygons-split-3857.zip` from https://osmdata.openstreetmap.de/data/water-polygons.html and save it to `$RVT_SHP_DIR` directory. 
2. Rename `shapefiles.<ext>` to `ocean0_0_0.<ext>`.
3. Split shapefiles up to zoom level 7:
```sh
cd $RVT_SHP_DIR

rsgeotools-subdiv-shapefile.sh 0 0 0 1 ocean

rsgeotools-subdiv-shapefile.sh 0 0 0 2 ocean
rm ocean1*

rsgeotools-subdiv-shapefile.sh 0 0 0 3 ocean
rm ocean2*

rsgeotools-subdiv-shapefile.sh 0 0 0 4 ocean
rm ocean3*

rsgeotools-subdiv-shapefile.sh 0 0 0 5 ocean
rm ocean4*

rsgeotools-subdiv-shapefile.sh 0 0 0 6 ocean
rm ocean5*

rsgeotools-subdiv-shapefile.sh 0 0 0 7 ocean
rm ocean6*
```

4. Split shapefiles up to zoom level 11 and pack them into .tar.gz for further usage:
```sh
rsgeotools-process-mass-subdiv-shapefile.sh 0 0 0 7 ocean
```

You will need to set `$RVT_SHP_ARCHIVE_DIR` (destination dir), `$RVT_SHP_DIR` (source dir, see above) and `$RVT_TEMP_DIR` (usually /tmp) environment variables.



