
import { Redis } from '@upstash/redis';

export async function GET() {
  const redisUrl =
    process.env.KV_REST_API_URL ||
    process.env.UPSTASH_REDIS_REST_URL ||
    process.env.LUDUS_REDIS_REST_URL;

  const redisToken =
    process.env.KV_REST_API_TOKEN ||
    process.env.UPSTASH_REDIS_REST_TOKEN ||
    process.env.LUDUS_REDIS_REST_TOKEN;

  const sessionSecretConfigured =
    (process.env.LUDUS_SESSION_SECRET || '').length >= 32;

  let redisConnected = false;
  let redisError: string | undefined;

  if (redisUrl && redisToken) {
    try {
      const redis = new Redis({
        url: redisUrl,
        token: redisToken,
        signal: () => AbortSignal.timeout(4000),
      });

      redisConnected = (await redis.ping()) === 'PONG';
    } catch (error) {
      redisError =
        error instanceof Error ? error.message : 'Redis ping failed';
    }
  }

  const configured = sessionSecretConfigured && redisConnected;

  return Response.json(
    {
      service: 'snoresaber-ludus',
      status: configured ? 'ready' : 'misconfigured',
      websocketPath: '/v1/connect',
      checks: {
        sessionSecretConfigured,
        redisCredentialsConfigured: Boolean(redisUrl && redisToken),
        redisConnected,
        ...(redisError ? { redisError } : {}),
      },
    },
    {
      status: configured ? 200 : 503,
      headers: { 'cache-control': 'no-store' },
    },
  );
}
