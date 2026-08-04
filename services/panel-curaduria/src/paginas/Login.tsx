import { useState } from 'react';
import { Navigate } from 'react-router-dom';
import { useSesion } from '../auth/sesion';
import { Aviso } from '../componentes/Aviso';

export function Login() {
  const { usuario, entrar } = useSesion();
  const [email, setEmail] = useState('');
  const [password, setPassword] = useState('');
  const [error, setError] = useState<string | null>(null);
  const [enviando, setEnviando] = useState(false);

  if (usuario) return <Navigate to="/especies" replace />;

  async function enviar(evento: React.FormEvent) {
    evento.preventDefault();
    setError(null);
    setEnviando(true);
    try {
      await entrar(email, password);
    } catch (fallo) {
      setError(fallo instanceof Error ? fallo.message : 'No se pudo iniciar sesión');
    } finally {
      setEnviando(false);
    }
  }

  return (
    <div className="login">
      <form onSubmit={enviar}>
        <h1>Curaduría</h1>
        <p className="ayuda">
          Biodiversidad de Chiloé. Entra con la misma cuenta que usas en la app.
        </p>

        <label>
          Correo
          <input
            type="email"
            value={email}
            onChange={(e) => setEmail(e.target.value)}
            autoComplete="username"
            required
          />
        </label>

        <label>
          Contraseña
          <input
            type="password"
            value={password}
            onChange={(e) => setPassword(e.target.value)}
            autoComplete="current-password"
            required
          />
        </label>

        {error && <Aviso tono="error">{error}</Aviso>}

        <button type="submit" disabled={enviando}>
          {enviando ? 'Entrando…' : 'Entrar'}
        </button>
      </form>
    </div>
  );
}
