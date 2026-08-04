export type TonoAviso = 'error' | 'exito' | 'atencion';

export function Aviso({ tono, children }: { tono: TonoAviso; children: React.ReactNode }) {
  return (
    <p className={`aviso aviso--${tono}`} role={tono === 'error' ? 'alert' : 'status'}>
      {children}
    </p>
  );
}
