def get_service(file_path):
    # List to store extracted service names
    service_names = []

    # Open and read the file
    with open(file_path, "r") as file:
        lines = file.readlines()

        # Iterate through each line in the file
        for line in lines:
            line = line.strip()  # Remove leading/trailing whitespace
            if line.startswith("service"):  # Check if the line starts with "service"
                parts = line.split()  # Split the line into words
                if len(parts) > 1:  # Ensure there's a word after "service"
                    service_name = parts[1].rstrip("{")  # Get the service name
                    service_names.append(service_name)


    #return the list of services
    return service_names[0]