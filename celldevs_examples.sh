echo Running asymmetric Cell-DEVS example...
cd example/celldevs_asymm_sir || exit
../../bin/main_asymm_sir config.json
cd ../..
echo Simulation done. Results are available in example/celldevs_asymm_sir/log.csv

echo Running classic Cell-DEVS example...
cd example/celldevs_grid_sir || exit
../../bin/main_grid_sir config.json
cd ../..
echo Simulation done. Results are available in example/celldevs_grid_sir/log.csv
