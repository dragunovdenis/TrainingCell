Timeout /t 1
TrainingEngineConsole.exe 1 --source script.txt --episodes 1000 --output "out_folder" --dump_rounds 1 --lambda_flag 1 --discount_flag 1 --min_simplex 0.001
pause