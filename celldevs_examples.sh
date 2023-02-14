echo Running asymmetric Cell-DEVS example...
cd example/celldevs_sir || exit
../../bin/main_asymm_sir asymm_config.json
echo Simulation done. Results are available in example/celldevs_sir/log_asymm_sir.csv

echo Running grid Cell-DEVS example...
../../bin/main_grid_sir grid_config.json
cd ../..
echo Simulation done. Results are available in example/celldevs_sir/log_grid_sir.csv
