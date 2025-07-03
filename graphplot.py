import matplotlib.pyplot as plt  # for creating plots and visualizing data

# sample data points found after running code for below q values. 
# each pair (q, m_max) represents max parties allowed before protocol breaks
q_values = [1153, 19457, 724481, 1496833]
m_max_values = [4, 14, 100, 100]

# Create a new figure with fixed dimensions
plt.figure(figsize=(8, 5))

# Plot q vs m_max as a blue line with dots
plt.plot(q_values, m_max_values, marker='o', linestyle='-', color='royalblue', label='Max m vs q')
plt.xlabel('Modulus q', fontsize=12)
plt.ylabel('Maximum m (before protocol breaks)', fontsize=12)
plt.title('q vs m_max for AVP Protocol', fontsize=14)
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.show()
