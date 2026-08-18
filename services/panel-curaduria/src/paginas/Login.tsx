import { useCallback, useEffect, useRef, useState } from 'react';
import { Navigate } from 'react-router-dom';
import { useSesion } from '../auth/sesion';
import { cargarGoogleIdentity, googleClientId } from '../auth/google';
import { Aviso } from '../componentes/Aviso';

export function Login() {
  const { usuario, entrar, entrarConGoogle } = useSesion();
  const [email, setEmail] = useState('');
  const [password, setPassword] = useState('');
  const [error, setError] = useState<string | null>(null);
  const [enviando, setEnviando] = useState(false);
  const [googleListo, setGoogleListo] = useState(false);
  const contenedorGoogle = useRef<HTMLDivElement | null>(null);

  // El callback de Google vive fuera de React: se guarda en una ref para que el
  // botón se dibuje una sola vez y aun así vea el estado actual.
  const alRecibirCredencial = useRef<(credential: string) => void>(() => {});
  alRecibirCredencial.current = (credential: string) => {
    setError(null);
    setEnviando(true);
    entrarConGoogle(credential)
      .catch((fallo: unknown) => {
        setError(fallo instanceof Error ? fallo.message : 'No se pudo entrar con Google');
      })
      .finally(() => setEnviando(false));
  };

  const montarBoton = useCallback(() => {
    let vigente = true;
    cargarGoogleIdentity()
      .then((id) => {
        if (!vigente || !contenedorGoogle.current) return;
        id.initialize({
          client_id: googleClientId,
          callback: (respuesta) => {
            if (respuesta.credential) alRecibirCredencial.current(respuesta.credential);
          },
          cancel_on_tap_outside: true,
        });
        id.renderButton(contenedorGoogle.current, {
          theme: 'outline',
          size: 'large',
          text: 'continue_with',
          locale: 'es',
        });
        setGoogleListo(true);
      })
      .catch(() => {
        // Que Google no cargue no debe romper el formulario de correo y clave.
        if (vigente) setGoogleListo(false);
      });
    return () => {
      vigente = false;
    };
  }, []);

  useEffect(montarBoton, [montarBoton]);

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

        <div className="login-google" ref={contenedorGoogle} />
        {!googleListo && (
          <p className="ayuda">Si entraste a la app con Google, usa ese mismo botón aquí.</p>
        )}

        <div className="login-separador">
          <span>o con correo</span>
        </div>

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
