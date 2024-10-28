import React, { useState } from 'react';
import { Container, TextField, Button, Typography, Box } from '@mui/material';

const ImpTerm = () => {
  // State for input fields
  const [newPIN, setNewPIN] = useState('');
  const [confirmPIN, setPINconfirmation] = useState('');

  // Handle submit
  const handleSubmit = (e) => {
    e.preventDefault();
    console.log('New access PIN:', newPIN);
    console.log('PIN confirmation:', confirmPIN);
  };

  return (
    <Container maxWidth="sm" style={{ marginTop: '50px' }}>
      <Box 
        component="form" 
        onSubmit={handleSubmit} 
        display="flex" 
        flexDirection="column" 
        gap={2}
      >
        <Typography variant="h4" align="center" gutterBottom>
          IMP Access Terminal
        </Typography>
        <TextField
          label="New access PIN"
          variant="outlined"
          value={newPIN}
          onChange={(e) => setNewPIN(e.target.value)}
          required
          type="number"
        />
        <TextField
          label="PIN confirmation"
          variant="outlined"
          value={confirmPIN}
          onChange={(e) => setPINconfirmation(e.target.value)}
          required
          type="number"
        />
        <Button 
          type="submit" 
          variant="contained" 
          color="primary"
          fullWidth
        >
          Change
        </Button>
      </Box>
    </Container>
  );
};

export default ImpTerm;
